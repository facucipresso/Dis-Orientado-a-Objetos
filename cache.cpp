#include <iostream>
#include <fstream>
#include <map>
#include <utility>
#include <string>
#include <vector>
using namespace std;

// encapsul datos (clave y objeto) para que se me haga mas facil la escritura y lectura desde un archivo binario
template <class T>
class DatoFile {
private:
    int key; 
    T obj;

public:
    DatoFile() : key(0), obj(T()) {}
    DatoFile(string _key, T _obj) : key(stoi(_key)), obj(_obj) {}
    ~DatoFile() {}

    string getKey() {
        return to_string(key);
    }

    T getObj() {
        return obj;
    }
};

template <class T>
class CacheManager {
private:
    int capacity;
    map<string, pair<T, int>> cache_data; // <Clave, <Obj, Indice de Uso>>
    int MRU = 1;
    const string nameFile = "archivo.dat";

    bool write_file(string key, T obj);
    T get_file(string key);
    bool update_file(string key, T obj);
    bool keyInFile(string key);

public:
    CacheManager(int); // recibe la capacidad en el int
    ~CacheManager() {}

    void insert(string key, T obj);
    T get(string key);
    void show_cache();
    string getLRU();
    void update_cache(string key, T obj);
};

template <class T>
bool CacheManager<T>::keyInFile(string key) {
    ifstream file(nameFile, ios::in | ios::binary);

    if (!file) {
        cerr << "Fallo de archivo en keyInFile" << endl;
        return false;
    }

    DatoFile<T> datoRead;
    // convertir el puntero a objeto DatoFile<T> en un puntero a un array de char, para leer o escribir el objeto en un archivo binario.
    while (file && file.read(reinterpret_cast<char*>(&datoRead), sizeof(datoRead))) {
        if (datoRead.getKey() == key) {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}


template <class T>
bool CacheManager<T>::update_file(string key, T obj) {
    fstream file(nameFile, ios::in | ios::out | ios::binary);

    if (!file) {
        cerr << "Fallo de archivo en update_file"; // indicador de fallo
        exit(EXIT_FAILURE);
    }

    vector<DatoFile<T>> datos;
    DatoFile<T> datoRead;

    while (file && file.read(reinterpret_cast<char*>(&datoRead), sizeof(datoRead))) {
        if (datoRead.getKey() != key) {
            datos.push_back(datoRead);
        }
    }

    file.close();
    file.open(nameFile, ios::out | ios::binary | ios::trunc); 

    if (!file) {
        cerr << "Fallo de archivo, reabriendo"; // indicador de fallo
        exit(EXIT_FAILURE);
    }

    for (auto dato : datos) {
        file.write(reinterpret_cast<const char*>(&dato), sizeof(dato));
    }

    file.close();
    return true;
}

template <class T>
bool CacheManager<T>::write_file(string key, T obj) {
    if (!keyInFile(key)) {
        cout << "Key " << key << " no encontrada en archivo, crando." << endl;
    } else {
        cout << "Key " << key << " encontrada en archivo, actualizando." << endl;
        update_file(key, obj);
    }

    DatoFile<T> d(key, obj);

    // Crear el archivo si no existe
    ofstream file(nameFile, ios::app | ios::binary);

    if (!file) {
        cerr << "Fallo de archivo en write_file" << endl; // indicador de fallo
        return false;
    }

    file.write(reinterpret_cast<const char*>(&d), sizeof(d));
    if (!file) {
        cerr << "Fallo de archivo: no se pueden escribir datos en write_file()" << endl; // indicador de fallo
        return false; 
    }
    file.close();
    return true;
}


template <class T>
T CacheManager<T>::get_file(string key) {
    ifstream file(nameFile, ios::in | ios::binary);

    if (!file) {
        cerr << "Fallo de archivo en get_file"; // indicador de fallo
        exit(EXIT_FAILURE);
    }

    DatoFile<T> datoRead;

    while (file && file.read(reinterpret_cast<char*>(&datoRead), sizeof(datoRead))) {
        if (datoRead.getKey() == key) {
            update_cache(datoRead.getKey(), datoRead.getObj());
            file.close();
            return datoRead.getObj();
        }
    }

    file.close();

    cerr << endl << "Objeto no existente" << endl; // si el objeto no existe
    T porDefault; 
    return porDefault;
}

template <class T>
string CacheManager<T>::getLRU() {
    auto it = cache_data.begin();

    string keyLRU = it->first;
    int LRU = it->second.second;

    while (it != cache_data.end()) {
        if (it->second.second < LRU) {
            LRU = it->second.second;
            keyLRU = it->first;
        }

        it++;
    }

    return keyLRU;
}

template <class T>
void CacheManager<T>::show_cache() {
    cout << endl << "Cache:" << endl;
    if (!cache_data.empty()) {
        for (auto& it : cache_data)
            cout << it.second.first << endl;
    } else {
        cout << "La cache se encuentra vacia";
    }
}

template <class T>
CacheManager<T>::CacheManager(int cap) {
    capacity = cap;
}

template <class T>
void CacheManager<T>::insert(string key, T obj) {
    cout << "Insertando key " << key << " en cache y en el archivo." << endl;
    update_cache(key, obj);
    write_file(key, obj);
}

template <class T>
void CacheManager<T>::update_cache(string key, T obj) {
    auto it = cache_data.find(key);

    if (cache_data.size() == capacity && it == cache_data.end()) {
        cache_data.erase(getLRU());
    }
    cache_data[key] = make_pair(obj, MRU);
    MRU++;
}

template <class T>
T CacheManager<T>::get(string key) {
    auto it = cache_data.find(key);

    if (it != cache_data.end()) {
        update_cache(it->first, it->second.first);
        return it->second.first;
    }

    return get_file(key);
}

class Student {
private:
    int id;
    int value;
    const char* data;

public:
    static const string class_name; // Cada clase tiene un static llamado : class_name
    Student() : id(0), value(0), data("") {}
    Student(int _key, int _value, const char* _data) : id(_key), value(_value), data(_data) {}
    ~Student() {}

    friend ostream& operator<<(ostream& os, Student s) {
        os << "Student Object: " << s.id << " , " << s.value << " , " << s.data << endl;
        return os;
    }
};

const string Student::class_name = "StudentClass";

int main() {
    
    //arranco cache con capacidad para tres elementos
    CacheManager<Student> my_cache(3);

    // prueba de carga
    my_cache.insert("0", Student(0, 22, "student1"));
    my_cache.insert("1", Student(1, 23, "student2"));
    my_cache.insert("2", Student(2, 24, "student3"));
    my_cache.insert("3", Student(3, 25, "student4"));

    my_cache.show_cache();

    cout << "-------------- Actualizo ---------------------" << endl;

    my_cache.insert("3", Student(6, 29, "student5"));

    my_cache.show_cache();

    cout << "--------Traigo un dato que no esta en cache --------" << endl;

    Student return_obj = my_cache.get("0");
    cout<<return_obj;

    cout << "\n----- Ni en cache ni el memoria-----" << endl;

    Student return_obj2 = my_cache.get("13");
    cout<<return_obj2; 

    cout << "--------------------------------------------------------" << endl;

    my_cache.insert("2", Student(22, 204, "student32"));
    my_cache.show_cache(); 

    my_cache.insert("9", Student(1, 5, "student50"));
    my_cache.insert("9", Student(1, 5, "student50"));
    my_cache.insert("9", Student(1, 5, "student50"));

    my_cache.show_cache();
    



    return 0;
}

