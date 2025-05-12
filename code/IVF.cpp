#include "IVF.h"
#include <algorithm>
#include <cmath>
#include <numeric>

using namespace std;
struct Candidate {
	int id;
	double dist;
	Candidate(int i, double d) : id(i), dist(d) {}
	bool operator<(const Candidate& other) const { return dist < other.dist; }
};

IVFIndex::IVFIndex(int nlist_, int nprobe_, int max_iter_)
	: nlist(nlist_), nprobe(nprobe_),
	kmeans(nlist_, max_iter_, false), data() {}

void IVFIndex::build(const DataSet& data_) {
	data = data_;
	kmeans.fit(data_);
	centroids = kmeans.get_centroids();
	build_inverted_lists(data_);
	deleted_ids_.clear();
}

void IVFIndex::build_inverted_lists(const DataSet& data) {
	inverted_lists_.resize(nlist);
	const auto& labels = kmeans.get_labels();
	
	for (size_t i = 0; i < data.size(); ++i) {
		if (deleted_ids_.count(i)) continue;
		int cluster_id = labels[i];
		if (cluster_id >= 0 && cluster_id < nlist) {
			inverted_lists_[cluster_id].push_back(i);
		}
	}
}

std::vector<int> IVFIndex::find_nearest_clusters(const Vector& query, int nprobe) const {
	std::priority_queue<Candidate> pq;
	
	for (int c = 0; c < nlist; ++c) {
		if (inverted_lists_[c].empty()) continue;
		double dist = KMeans::distance(query, centroids[c]);
		pq.emplace(c, dist);
		if (pq.size() > nprobe) pq.pop();
	}
	
	std::vector<int> result;
	while (!pq.empty()) {
		result.push_back(pq.top().id);
		pq.pop();
	}
	std::reverse(result.begin(), result.end());
	return result;
}

std::vector<int> IVFIndex::search(const Vector& query, int k) const {
	auto cluster_ids = find_nearest_clusters(query, nprobe);
	std::priority_queue<Candidate> pq;
	
	for (int c : cluster_ids) {
		for (int vec_id : inverted_lists_[c]) {
			if (deleted_ids_.count(vec_id)) continue;
			double dist = KMeans::distance(query, data[vec_id]);
			pq.emplace(vec_id, dist);
			if (pq.size() > k) pq.pop();
		}
	}
	
	std::vector<int> result;
	while (!pq.empty()) {
		result.push_back(pq.top().id);
		pq.pop();
	}
	std::reverse(result.begin(), result.end());
	return result;
}

void IVFIndex::insert(const Vector& vec) {
	int nearest_cluster = -1;
	double min_dist = numeric_limits<double>::max();
	for (int c = 0; c < nlist; ++c) {
		if (inverted_lists_[c].empty()) continue;
		double dist = KMeans::distance(vec, centroids[c]);
		if (dist < min_dist) {
			min_dist = dist;
			nearest_cluster = c;
		}
	}

	if (nearest_cluster == -1) {
		nearest_cluster = 0;
		centroids[0] = vec;
	}

	data.push_back(vec);
	int new_id = data.size() - 1;
	inverted_lists_[nearest_cluster].push_back(new_id);
	update_centroid(nearest_cluster);
}

void IVFIndex::remove(int vec_id) {
	if (vec_id < 0 || vec_id >= static_cast<int>(data.size())) return;
	
	deleted_ids_.insert(vec_id);
	centroids_updated_ = false;

	if (deleted_ids_.size() > data.size() / 100) {
		incremental_reassign();
	}
}

void IVFIndex::update_centroid(int cluster_id) {
	Vector new_centroid(centroids[0].size(), 0.0);
	int count = 0;
	
	for (int vec_id : inverted_lists_[cluster_id]) {
		if (deleted_ids_.count(vec_id)) continue;
		for (size_t i = 0; i < data[vec_id].size(); ++i) {
			new_centroid[i] += data[vec_id][i];
		}
		count++;
	}
	
	if (count > 0) {
		for (auto& val : new_centroid) val /= count;
		centroids[cluster_id] = new_centroid;
	}
}

void IVFIndex::incremental_reassign() {
	DataSet valid_data;
	vector<int> id_map(data.size());
	
	int new_id = 0;
	for (size_t i = 0; i < data.size(); ++i) {
		if (!deleted_ids_.count(i)) {
			valid_data.push_back(data[i]);
			id_map[i] = new_id++;
		}
	}
	
	for (auto& list : inverted_lists_) {
		vector<int> new_list;
		for (int id : list) {
			if (!deleted_ids_.count(id)) {
				new_list.push_back(id_map[id]);
			}
		}
		list = new_list;
	}
	
	data = valid_data;
	deleted_ids_.clear();
	
	kmeans.fit(data);
	centroids = kmeans.get_centroids();
}

void IVFIndex::split_cluster(int cluster_id) {
	validate_cluster(cluster_id);
	if (inverted_lists_[cluster_id].size() < 2) return;

	DataSet valid_data;
	for (int vec_id : inverted_lists_[cluster_id]) {
		if (!deleted_ids_.count(vec_id)) {
			valid_data.push_back(data[vec_id]);
		}
	}

	KMeans sub_kmeans(2, 50, false);
	sub_kmeans.fit(valid_data);
	auto& sub_labels = sub_kmeans.get_labels();

	nlist++;
	centroids.push_back(sub_kmeans.get_centroids()[1]);
	inverted_lists_.emplace_back();

	vector<int> new_list;
	size_t label_idx = 0;
	for (int vec_id : inverted_lists_[cluster_id]) {
		if (deleted_ids_.count(vec_id)) continue;
		if (sub_labels[label_idx++] == 0) {
			new_list.push_back(vec_id);
		} else {
			inverted_lists_.back().push_back(vec_id);
		}
	}
	inverted_lists_[cluster_id] = new_list;
	centroids[cluster_id] = sub_kmeans.get_centroids()[0];
}

void IVFIndex::merge_clusters(int cluster_id1, int cluster_id2) {
	validate_cluster(cluster_id1);
	validate_cluster(cluster_id2);

	inverted_lists_[cluster_id1].insert(
		inverted_lists_[cluster_id1].end(),
		inverted_lists_[cluster_id2].begin(),
		inverted_lists_[cluster_id2].end()
	);
	
	update_centroid(cluster_id1);
	
	inverted_lists_[cluster_id2].clear();
	centroids.erase(centroids.begin() + cluster_id2);
	inverted_lists_.erase(inverted_lists_.begin() + cluster_id2);
	nlist--;
}

void IVFIndex::validate_cluster(int cluster_id) {
	if (cluster_id < 0 || cluster_id >= nlist) {
		throw runtime_error("Invalid cluster ID");
	}
}