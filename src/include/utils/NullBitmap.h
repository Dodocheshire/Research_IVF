#ifndef _NULL_BITMAP_H_
#define _NULL_BITMAP_H_

#include <vector>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <limits>

class NullBitmap {

public:
    NullBitmap() = default;
    
    /**
     * @brief 创建指定大小的bitmap，初始全部为非null
     */
    explicit NullBitmap(uint32_t size) {
        resize(size);
    }

    /**
     * @brief 将第index位设置为bit1，即该向量设置为删除状态
     * @attention 如果index超过size，会触发assert failed
     */
    void set(int index) {
        assert(index >= 0 && index < size_);
        bitmap_[index/8] |= (1 << (index%8));
    }

    /**
     * @brief 扩展到至少new_size大小，0填充扩展项
     */
    void resize(uint32_t new_size) {
        if(new_size <= size_) return;
        size_ = new_size;
        bitmap_.resize((size_ + 7) / 8, 0);
    }

    /**
     * @brief 清除null 标志，恢复为有效状态
     */
    void clear(int index) {
        assert(index >= 0 && index < size_);
        bitmap_[index/8] &= ~(1 << (index%8));
    }

    /**
     * @brief 查询是否为 null
     */
    bool is_null(int index) const {
        assert(index >= 0 && index < size_);
        return (bitmap_[index/8] >> (index%8)) & 1; 
    }

    uint32_t count_nulls() const {
        uint32_t count = 0;
        for (uint32_t i = 0; i < size_; ++i) {
            if (is_null(i)) ++count;
        }
        return count;
    }

    int find_first_valid() const {
        for (uint32_t i = 0; i < size_; ++i) {
            if (!is_null(i)) return static_cast<int>(i);
        }
        return -1; // 全为 null
    }

    /**
     * @brief resize到恰好能容纳size_个bit的状态
     */
    void compress() {
        bitmap_.resize((size_ + 7) / 8);
    }

    /**
     * @brief 压缩到最小空间后序列化到输出文件流中: |size_(4)| |null bitmap((size_ + 7) / 8)|
     * @attention (size_ + 7) / 8 是bitmap所占的byte数量
     */
    void serializeTo(std::ostream &os) {
        compress();
        os.write(reinterpret_cast<const char*>(&size_), sizeof(size_));
        os.write(reinterpret_cast<const char*>(bitmap_.data()), bitmap_.size());
    }

    /**
     * @brief 从输入文件流中反序列化到nullbitmap中
     */
    void deserializeFrom(std::istream &is) {
        uint32_t new_size;
        is.read(reinterpret_cast<char*>(&new_size), sizeof(new_size));
        size_ = new_size;
        compress();
        is.read(reinterpret_cast<char*>(bitmap_.data()), bitmap_.size());
    }
    /**
     * @brief bitmap管理的逻辑大小
     */
    uint32_t size() const {
        return size_;
    }

private:
    uint32_t size_ = 0;
    std::vector<uint8_t> bitmap_;

};

#endif