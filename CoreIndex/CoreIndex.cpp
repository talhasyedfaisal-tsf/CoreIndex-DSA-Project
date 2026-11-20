#include "CoreIndex.h"
#include <iostream>
#include <cstddef>

using namespace std;

static void copyText(char* destination, const char* source, int limit) {
    int i = 0;
    while (i + 1 < limit && source[i] != '\0') {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = '\0';
}

static int compareText(const char* first, const char* second) {
    int i = 0;
    while (first[i] != '\0' && second[i] != '\0' && first[i] == second[i]) ++i;
    if (first[i] == second[i]) return 0;
    return static_cast<unsigned char>(first[i]) < static_cast<unsigned char>(second[i]) ? -1 : 1;
}

HashTable::HashTable(int tableCapacity) : capacity(tableCapacity) {
    buckets = new HashNode*[static_cast<size_t>(capacity)];
    for (int i = 0; i < capacity; ++i) buckets[i] = nullptr;
}

HashTable::~HashTable() {
    for (int i = 0; i < capacity; ++i) {
        HashNode* current = buckets[i];
        while (current) {
            HashNode* next = current->next;
            delete current;
            current = next;
        }
    }
    delete[] buckets;
}

unsigned int HashTable::hash(int id) const {
    unsigned int value = static_cast<unsigned int>(id);
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    return value % static_cast<unsigned int>(capacity);
}

bool HashTable::insert(Record* record) {
    if (!record || find(record->id)) return false;
    unsigned int index = hash(record->id);
    HashNode* node = new HashNode;
    node->record = record;
    node->next = buckets[index];
    buckets[index] = node;
    return true;
}

Record* HashTable::find(int id) const {
    unsigned int index = hash(id);
    HashNode* current = buckets[index];
    while (current) {
        if (current->record->id == id) return current->record;
        current = current->next;
    }
    return nullptr;
}

bool HashTable::remove(int id) {
    unsigned int index = hash(id);
    HashNode* current = buckets[index];
    HashNode* previous = nullptr;
    while (current) {
        if (current->record->id == id) {
            if (previous) previous->next = current->next;
            else buckets[index] = current->next;
            delete current;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

AvlCategoryIndex::AvlCategoryIndex() : root(nullptr) {}
AvlCategoryIndex::~AvlCategoryIndex() { destroy(root); }

int AvlCategoryIndex::height(AvlNode* node) const {
    return node ? node->height : 0;
}

int AvlCategoryIndex::balance(AvlNode* node) const {
    return node ? height(node->left) - height(node->right) : 0;
}

AvlNode* AvlCategoryIndex::rotateRight(AvlNode* y) {
    AvlNode* x = y->left;
    AvlNode* temporary = x->right;
    x->right = y;
    y->left = temporary;
    y->height = 1 + (height(y->left) > height(y->right) ? height(y->left) : height(y->right));
    x->height = 1 + (height(x->left) > height(x->right) ? height(x->left) : height(x->right));
    return x;
}

AvlNode* AvlCategoryIndex::rotateLeft(AvlNode* x) {
    AvlNode* y = x->right;
    AvlNode* temporary = y->left;
    y->left = x;
    x->right = temporary;
    x->height = 1 + (height(x->left) > height(x->right) ? height(x->left) : height(x->right));
    y->height = 1 + (height(y->left) > height(y->right) ? height(y->left) : height(y->right));
    return y;
}

AvlNode* AvlCategoryIndex::insertNode(AvlNode* node, Record* record) {
    if (!node) {
        AvlNode* created = new AvlNode;
        copyText(created->category, record->category, 32);
        created->records = record;
        record->nextInCategory = nullptr;
        created->left = created->right = nullptr;
        created->height = 1;
        return created;
    }

    int comparison = compareText(record->category, node->category);
    if (comparison < 0) node->left = insertNode(node->left, record);
    else if (comparison > 0) node->right = insertNode(node->right, record);
    else {
        record->nextInCategory = node->records;
        node->records = record;
        return node;
    }

    node->height = 1 + (height(node->left) > height(node->right) ? height(node->left) : height(node->right));
    int factor = balance(node);
    if (factor > 1 && compareText(record->category, node->left->category) < 0) return rotateRight(node);
    if (factor < -1 && compareText(record->category, node->right->category) > 0) return rotateLeft(node);
    if (factor > 1 && compareText(record->category, node->left->category) > 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (factor < -1 && compareText(record->category, node->right->category) < 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

void AvlCategoryIndex::insert(Record* record) {
    root = insertNode(root, record);
}

AvlNode* AvlCategoryIndex::findMin(AvlNode* node) {
    while (node && node->left) node = node->left;
    return node;
}

AvlNode* AvlCategoryIndex::removeNode(AvlNode* node, const char* category) {
    if (!node) return nullptr;
    int comparison = compareText(category, node->category);
    if (comparison < 0) node->left = removeNode(node->left, category);
    else if (comparison > 0) node->right = removeNode(node->right, category);
    else {
        if (!node->left || !node->right) {
            AvlNode* temporary = node->left ? node->left : node->right;
            delete node;
            return temporary;
        }
        AvlNode* successor = findMin(node->right);
        copyText(node->category, successor->category, 32);
        node->records = successor->records;
        node->right = removeNode(node->right, successor->category);
    }

    if (!node) return nullptr;
    node->height = 1 + (height(node->left) > height(node->right) ? height(node->left) : height(node->right));
    int factor = balance(node);
    if (factor > 1 && balance(node->left) >= 0) return rotateRight(node);
    if (factor > 1 && balance(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (factor < -1 && balance(node->right) <= 0) return rotateLeft(node);
    if (factor < -1 && balance(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

bool AvlCategoryIndex::remove(Record* record) {
    AvlNode* node = root;
    while (node) {
        int comparison = compareText(record->category, node->category);
        if (comparison < 0) node = node->left;
        else if (comparison > 0) node = node->right;
        else break;
    }
    if (!node) return false;

    Record* previous = nullptr;
    Record* current = node->records;
    while (current) {
        if (current == record) {
            if (previous) previous->nextInCategory = current->nextInCategory;
            else node->records = current->nextInCategory;
            current->nextInCategory = nullptr;
            if (!node->records) root = removeNode(root, node->category);
            return true;
        }
        previous = current;
        current = current->nextInCategory;
    }
    return false;
}

void AvlCategoryIndex::destroy(AvlNode* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

void AvlCategoryIndex::printOrdered(AvlNode* node) const {
    if (!node) return;
    printOrdered(node->left);
    for (Record* record = node->records; record; record = record->nextInCategory) {
        cout << "  [" << record->id << "] " << record->name
             << " | priority: " << record->priority << endl;
    }
    printOrdered(node->right);
}

void AvlCategoryIndex::printOrdered() const {
    printOrdered(root);
}

void AvlCategoryIndex::printRange(AvlNode* node, const char* low, const char* high) const {
    if (!node) return;
    if (compareText(low, node->category) < 0) printRange(node->left, low, high);
    if (compareText(node->category, low) >= 0 && compareText(node->category, high) <= 0) {
        cout << "Category: " << node->category << endl;
        for (Record* record = node->records; record; record = record->nextInCategory) {
            cout << "  [" << record->id << "] " << record->name
                 << " | priority: " << record->priority << endl;
        }
    }
    if (compareText(high, node->category) > 0) printRange(node->right, low, high);
}

void AvlCategoryIndex::printRange(const char* low, const char* high) const {
    printRange(root, low, high);
}

MaxHeap::MaxHeap(int initialCapacity) : size(0), capacity(initialCapacity) {
    data = new Record*[static_cast<size_t>(capacity)];
}

MaxHeap::~MaxHeap() { delete[] data; }

void MaxHeap::resize() {
    int newCapacity = capacity * 2;
    Record** newData = new Record*[static_cast<size_t>(newCapacity)];
    for (int i = 0; i < size; ++i) newData[i] = data[i];
    delete[] data;
    data = newData;
    capacity = newCapacity;
}

void MaxHeap::insert(Record* record) {
    if (size == capacity) resize();
    int index = size++;
    data[index] = record;
    siftUp(index);
}

void MaxHeap::siftUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (data[parent]->priority >= data[index]->priority) break;
        Record* temporary = data[parent];
        data[parent] = data[index];
        data[index] = temporary;
        index = parent;
    }
}

void MaxHeap::siftDown(int index) {
    while (true) {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        if (left < size && data[left]->priority > data[largest]->priority) largest = left;
        if (right < size && data[right]->priority > data[largest]->priority) largest = right;
        if (largest == index) break;
        Record* temporary = data[index];
        data[index] = data[largest];
        data[largest] = temporary;
        index = largest;
    }
}

bool MaxHeap::extractMax(Record*& record) {
    if (size == 0) return false;
    record = data[0];
    data[0] = data[--size];
    if (size > 0) siftDown(0);
    return true;
}

bool MaxHeap::remove(int id) {
    for (int i = 0; i < size; ++i) {
        if (data[i]->id == id) {
            data[i] = data[--size];
            if (i < size) {
                if (i > 0 && data[i]->priority > data[(i - 1) / 2]->priority) siftUp(i);
                else siftDown(i);
            }
            return true;
        }
    }
    return false;
}

int MaxHeap::getSize() const { return size; }

CoreIndex::CoreIndex() : hashIndex(new HashTable()), avlIndex(new AvlCategoryIndex()), priorityIndex(new MaxHeap()), count(0) {}

CoreIndex::~CoreIndex() {
    Record** records = new Record*[static_cast<size_t>(count)];
    for (int i = 0; i < count; ++i) records[i] = nullptr;
    int found = 0;
    while (priorityIndex->getSize() > 0) {
        Record* record = nullptr;
        if (priorityIndex->extractMax(record) && record) records[found++] = record;
    }
    for (int i = 0; i < found; ++i) delete records[i];
    delete[] records;
    delete priorityIndex;
    delete avlIndex;
    delete hashIndex;
}

bool CoreIndex::addRecord(int id, const char* name, const char* category, int priority) {
    if (id < 0 || priority < 0 || priority > 100) return false;
    if (!name || name[0] == '\0' || !category || category[0] == '\0') return false;
    if (hashIndex->find(id)) return false;
    Record* record = new Record;
    record->id = id;
    copyText(record->name, name, 64);
    copyText(record->category, category, 32);
    record->priority = priority;
    record->nextInCategory = nullptr;
    if (!hashIndex->insert(record)) {
        delete record;
        return false;
    }
    avlIndex->insert(record);
    priorityIndex->insert(record);
    ++count;
    return true;
}

Record* CoreIndex::findRecord(int id) const {
    return hashIndex->find(id);
}

bool CoreIndex::updateRecord(int id, const char* name, const char* category, int priority) {
    if (id < 0 || priority < 0 || priority > 100) return false;
    if (!name || name[0] == '\0' || !category || category[0] == '\0') return false;
    Record* record = hashIndex->find(id);
    if (!record) return false;

    bool categoryChanged = compareText(record->category, category) != 0;
    if (categoryChanged) avlIndex->remove(record);
    copyText(record->name, name, 64);
    copyText(record->category, category, 32);
    record->priority = priority;
    if (categoryChanged) avlIndex->insert(record);
    priorityIndex->remove(id);
    priorityIndex->insert(record);
    return true;
}

bool CoreIndex::deleteRecord(int id) {
    Record* record = hashIndex->find(id);
    if (!record) return false;
    avlIndex->remove(record);
    priorityIndex->remove(id);
    hashIndex->remove(id);
    delete record;
    --count;
    return true;
}

bool CoreIndex::dispatchNext(Record*& record) {
    Record* dispatched = nullptr;
    if (!priorityIndex->extractMax(dispatched)) return false;
    avlIndex->remove(dispatched);
    hashIndex->remove(dispatched->id);

    record = new Record;
    *record = *dispatched;
    record->nextInCategory = nullptr;
    delete dispatched;
    --count;
    return true;
}

void CoreIndex::printAllByCategory() const {
    avlIndex->printOrdered();
}

void CoreIndex::printCategoryRange(const char* low, const char* high) const {
    avlIndex->printRange(low, high);
}

int CoreIndex::recordCount() const { return count; }
