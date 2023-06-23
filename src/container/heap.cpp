/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2021-01-16 19:07:29
 * @modify date 2021-01-16 19:07:29
 * @desc [description]
 */

#include "heap.h"

using namespace container;

template <typename T, typename Compare>
BinaryHeap<T, Compare>::BinaryHeap()
{
    data.push_back(0);
};

template <typename T, typename Compare>
BinaryHeap<T, Compare>::~BinaryHeap(){};

template <typename T, typename Compare>
void BinaryHeap<T, Compare>::build(const std::vector<T> &vals)
{
    data.resize(vals.size()+1);
    std::copy(vals.begin(), vals.end(), data.begin()+1);

    int n = data.size();
    int id = parent(n-1);
    while (check(id))
    {
        heapDown(id);
        id--;
    }
}

template <typename T, typename Compare>
void BinaryHeap<T, Compare>::heapUP(int id)
{
    if (check(parent(id)) && check(id))
    {
        while (compare(data[id], data[parent(id)]))
        {
            swap(id, parent(id));
            id = parent(id);
        }
    }
}

template <typename T, typename Compare>
void BinaryHeap<T, Compare>::heapDown(int id)
{
    if (!check(id)) return;
    while (true)
    {
        int left_id = left(id);
        int right_id = right(id);
        if (check(left_id) && check(right_id))
        {
            if (compare(data[id], data[left_id]) && compare(data[id], data[right_id]))
            {
                break;
            }
            else
            {
                if (compare(data[left_id], data[right_id]))
                {
                    swap(id, left_id);
                    id = left_id;
                }
                else
                {
                    swap(id, right_id);
                    id = right_id;
                }
            }
        }
        else
        {
            if (check(left_id))
            {
                if (compare(data[left_id], data[id]))
                {
                    swap(id, left_id);
                    id = left_id;
                }
            }
            break;
        }
    }
}

template <typename T, typename Compare>
void BinaryHeap<T, Compare>::insert(T val)
{
    data.push_back(val);

    int n = data.size();
    heapUP(n-1);
}

template <typename T, typename Compare>
void BinaryHeap<T, Compare>::remove(T val)
{
    int id = 0;
    int n = data.size();
    for (long int i=1; i<n; i++)
    {
        if (data[i] == val)
        {
            id = i;
            break;
        }
    }

    if (compare(data.back(), val) && check(id))
    {
        swap(id, n-1);
        data.pop_back();
        heapUP(id);
    }
    else
    {
        swap(id, n-1);
        data.pop_back();
        heapDown(id);
    }
}

template class BinaryHeap<float, GreaterEqual<float>>;
template class BinaryHeap<float, LessEqual<float>>;
