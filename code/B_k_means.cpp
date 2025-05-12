#include "B_k_means.h"
#include <iostream>
#include <limits>
#include <omp.h>
#include <random>

using namespace std;

// use k-means++ initialization
// use balanced k-means to generate balanced cluster sizes

KMeans::KMeans(int k_, int max_iter_, bool verbose_)
	: k(k_), max_iter(max_iter_), verbose(verbose_) {}

void KMeans::fit(const DataSet& data) {
	init_centroids(data);
	labels.resize(data.size());
	for (int it = 0; it < max_iter; it++) {
		#pragma omp parallel for
		for (size_t i = 0; i < data.size(); i++) {
			double min_dist = numeric_limits<double>::max();
			int min_index = 0;
			for (int j = 0; j < k; j++) {
				double dist = distance(data[i], centroids[j]);
				if (dist < min_dist) {
					min_dist = dist;
					min_index = j;
				}
			}
			labels[i] = min_index;
		}

		vector<Vector> new_centroids(k, Vector(data[0].size(), 0));
	    vector<int> count(k, 0);
	    bool changed = false;

	    for (size_t i = 0; i < data.size(); i++) {
            int cluster = labels[i];
            for (size_t j = 0; j < data[i].size(); j++) {
                new_centroids[cluster][j] += data[i][j];
            }
            count[cluster]++;
        }

        for (int c = 0; c < k; c++) {
            if (count[c] == 0) {
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> dis(0, data.size()-1);
                new_centroids[c] = data[dis(gen)];
                continue;
            }
            double scale = 1.0 / count[c];
            for (auto& val : new_centroids[c]) {
                val *= scale;
            }
            if (distance(centroids[c], new_centroids[c]) > 1e-12) {
                changed = true;
            }
        }
	    if (!changed) break;


		if (verbose) cout << "Iter " << it << " complete" << endl;
	}
}

void KMeans::init_centroids(const DataSet& data) {
    // random_device rd;
    // mt19937 gen(rd());
    // uniform_int_distribution<> dis(0, data.size() - 1);

    // centroids.clear();
    // centroids.reserve(k);
    // vector<bool> selected(data.size(), false);
    // for (int i = 0; i < k; ) {
    //     int index = dis(gen);
    //     if (!selected[index]) {
    //         centroids.push_back(data[index]);
    //         selected[index] = true;
    //         i++;
    //     }
    // }

    // k-means++
    if(data.empty()) return ;
    centroids.clear();
    random_device rd;
    mt19937 gen(rd());

    // 随机选择第一个质心
    
}

double KMeans::distance(const Vector& a, const Vector& b) {
	if (a.empty() || b.empty() || a.size() != b.size()) {
		throw invalid_argument("Vectors must be non-empty and same dimension");
	}
	double dist = 0;
	#pragma omp simd reduction(+:dist)
	for (size_t i = 0; i < a.size(); i++) {
		dist += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return dist;
}