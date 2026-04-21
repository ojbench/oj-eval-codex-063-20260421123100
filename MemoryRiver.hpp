#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>
#include <string>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>
class MemoryRiver {
private:
    // We reserve one extra int at the file start for a free-list head pointer
    // and map user-visible header ints after this extra slot.
    static constexpr int extra_header_ints = 1;

    fstream file;
    string file_name;
    int sizeofT = sizeof(T);

    std::streamoff header_bytes() const {
        return static_cast<std::streamoff>(sizeof(int)) * (info_len + extra_header_ints);
    }

    // Free list head is stored at int position 0 (before user-visible header)
    int get_free_head() {
        file.seekg(0, std::ios::beg);
        int head = 0;
        file.read(reinterpret_cast<char *>(&head), sizeof(int));
        return head;
    }

    void set_free_head(int head) {
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<char *>(&head), sizeof(int));
        file.flush();
    }

public:
    MemoryRiver() = default;

    MemoryRiver(const string& file_name) : file_name(file_name) {}

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        // Create/truncate binary file and write (info_len + extra) ints initialized to 0
        file.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
        int tmp = 0;
        for (int i = 0; i < info_len + extra_header_ints; ++i)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    // Read the n-th user-visible int (1-based) into tmp
    void get_info(int &tmp, int n) {
        if (n > info_len || n <= 0) return;
        file.open(file_name, std::ios::in | std::ios::binary);
        if (!file) { tmp = 0; return; }
        std::streamoff pos = static_cast<std::streamoff>(sizeof(int)) * (extra_header_ints + (n - 1));
        file.seekg(pos, std::ios::beg);
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    // Write tmp into the n-th user-visible int position (1-based)
    void write_info(int tmp, int n) {
        if (n > info_len || n <= 0) return;
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) return;
        std::streamoff pos = static_cast<std::streamoff>(sizeof(int)) * (extra_header_ints + (n - 1));
        file.seekp(pos, std::ios::beg);
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    // Write object t to a suitable position and return its starting offset index
    int write(T &t) {
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) return -1;

        int head = get_free_head();
        int index;
        if (head == 0) {
            // Append at the end
            file.seekp(0, std::ios::end);
            index = static_cast<int>(file.tellp());
        } else {
            // Reuse a free block
            index = head;
            // Read next pointer stored at this free block
            int next = 0;
            file.seekg(index, std::ios::beg);
            file.read(reinterpret_cast<char *>(&next), sizeof(int));
            set_free_head(next);
            // Position write pointer to index for writing T
            file.seekp(index, std::ios::beg);
        }

        // If appending, ensure write pointer at end (set after computing index)
        if (head == 0) file.seekp(index, std::ios::beg);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
        return index;
    }

    // Update object at given index
    void update(T &t, const int index) {
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) return;
        file.seekp(index, std::ios::beg);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
    }

    // Read object at given index into t
    void read(T &t, const int index) {
        file.open(file_name, std::ios::in | std::ios::binary);
        if (!file) return;
        file.seekg(index, std::ios::beg);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
    }

    // Delete object at index: push its block onto free list
    void Delete(int index) {
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) return;
        int head = get_free_head();
        // Store current head at the start of this block
        file.seekp(index, std::ios::beg);
        file.write(reinterpret_cast<char *>(&head), sizeof(int));
        // Update head to this index
        set_free_head(index);
        file.close();
    }
};


#endif //BPT_MEMORYRIVER_HPP

