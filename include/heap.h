/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2021-01-16 10:34:21
 * @modify date 2021-01-16 10:34:21
 * @desc [description]
 */

#ifndef HEAP_H
#define HEAP_H

#include <iostream>
#include <vector>

namespace container
{
    template<typename T>
    class GreaterEqual
    {
    public:
        double operator()(T x, T y)
        { 
            return x>=y;
        }
    };

    template<typename T>
    class LessEqual
    {
    public:
        double operator()(T x, T y)
        { 
            return x<=y;
        }
    };

    template <typename T, typename Compare>
    class BinaryHeap
    {
    public:
        BinaryHeap();
        ~BinaryHeap();
        void build(const std::vector<T> &vals);
        void heapUP(int id);
        void heapDown(int id);
        void insert(T val);
        void remove(T val);

        void print()
        {
            for (auto d=data.begin(); d!=data.end(); ++d)
            {
                std::cout<<*d<<" ";
            }
            std::cout<<std::endl;
        }
    private:
        int parent(int i)
        {
            return i>>1;
        }
        int left(int i)
        {
            return i<<1;
        }
        int right(int i)
        {
            return (i<<1)+1;
        }
        void swap(int i, int j)
        {
            T temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
        bool check(int id)
        {
            return id>0 && id<data.size();
        }
    public:
        std::vector<T> data;
    private:
        Compare compare;
    };
}

#endif /* HEAP_H */
