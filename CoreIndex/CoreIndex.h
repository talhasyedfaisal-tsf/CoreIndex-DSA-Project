#ifndef CORE_INDEX_H
#define CORE_INDEX_H

struct Record {
    int id;
    char name[64];
    char category[32];
    int priority;
    Record* nextInCategory;
};

struct HashNode {
    Record* record;
    HashNode* next;
};

class HashTable {
public:
    HashTable(int capacity = 101);
    ~HashTable();
    bool insert(Record* record);
    Record* find(int id) const;
    bool remove(int id);

private:
    HashNode** buckets;
    int capacity;
    unsigned int hash(int id) const;
};

struct AvlNode {
    char category[32];
    Record* records;
    AvlNode* left;
    AvlNode* right;
    int height;
};

class AvlCategoryIndex {
public:
    AvlCategoryIndex();
    ~AvlCategoryIndex();
    void insert(Record* record);
    bool remove(Record* record);
    void printOrdered() const;
    void printRange(const char* low, const char* high) const;

private:
    AvlNode* root;
    int height(AvlNode* node) const;
    int balance(AvlNode* node) const;
    AvlNode* rotateRight(AvlNode* y);
    AvlNode* rotateLeft(AvlNode* x);
    AvlNode* insertNode(AvlNode* node, Record* record);
    AvlNode* findMin(AvlNode* node);
    AvlNode* removeNode(AvlNode* node, const char* category);
    void destroy(AvlNode* node);
    void printOrdered(AvlNode* node) const;
    void printRange(AvlNode* node, const char* low, const char* high) const;
};

class MaxHeap {
public:
    MaxHeap(int initialCapacity = 16);
    ~MaxHeap();
    void insert(Record* record);
    bool extractMax(Record*& record);
    bool remove(int id);
    int getSize() const;

private:
    Record** data;
    int size;
    int capacity;
    void resize();
    void siftUp(int index);
    void siftDown(int index);
};

class CoreIndex {
public:
    CoreIndex();
    ~CoreIndex();

    bool addRecord(int id, const char* name, const char* category, int priority);
    Record* findRecord(int id) const;
    bool updateRecord(int id, const char* name, const char* category, int priority);
    bool deleteRecord(int id);
    bool dispatchNext(Record*& record);
    void printAllByCategory() const;
    void printCategoryRange(const char* low, const char* high) const;
    int recordCount() const;

private:
    HashTable* hashIndex;
    AvlCategoryIndex* avlIndex;
    MaxHeap* priorityIndex;
    int count;
};

#endif
