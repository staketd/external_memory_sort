#include <iostream>
#include <fstream>
#include <filesystem>
#include <climits>

void print_file(std::string filename) {
    size_t file_size = 1000*4;
    std::ifstream input(filename, std::ios_base::binary | std::ios_base::in);
    int array[file_size / 4];
    input.read((char *) &array, file_size);

    for (int i = 0; i < file_size / 4; ++i) {
        std::cout << array[i] << "\n";
    }
}

bool check_sorted(std::string filename) {
    size_t file_size = std::filesystem::file_size(filename);
    std::ifstream input(filename, std::ios_base::binary | std::ios_base::in);
    size_t read_batch_size = 10 * 1024 * 1024;
    int* array = new int[read_batch_size];
    int prev = INT_MIN;
    for (size_t i = 0; i < file_size; i += read_batch_size * 4) {
        size_t read_size = std::min((file_size - i) / 4, read_batch_size);
        input.read((char*)array, read_size * 4);
        for (size_t j = 0; j < read_size; ++j) {
            if (array[j] < prev) {
                return false;
            }
            prev = array[j];
        }
    }
    delete[] array;
    return true;
}

void write_file(std::string filename, int max_number) {
    std::ofstream out(filename, std::ios_base::binary | std::ios_base::out);

    size_t write_size = 32 * 1024;

    int* buffer = new int[write_size / 4];
    int number = 0;
    size_t writes = (max_number * 4 - 1) / write_size + 1;
    for (size_t write = 0; write < writes; ++write) {
        size_t numbers = std::min(max_number - write * write_size / 4, write_size / 4);

        for (size_t i = 0; i < numbers; ++i, ++number) {
            buffer[i] = number * (number % 2 == 0 ? -1 : 1);
        }

        out.write((char*) buffer, numbers * 4);
    }
}

int main(int argc, char** argv) {
    int max_number = std::atol(argv[1]);

    // print_file("tmp/0_128");

    write_file("numbers", max_number);
    // print_file("sorted_numbers");
    // std::cout << (check_sorted("sorted_numbers") ? "sorted" : "unsorted");
}