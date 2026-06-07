#include <bits/stdc++.h>

using namespace std;

const int N = 20;
const int INF = INT_MAX >> 2;


// (r, subset, next_vertice) = floid[U][V]
//  r - минимальное расстояние между вершинами U и V
//  subset - множество вершин, лежащие на пути между U и V
//  next_vertice - следующая вершина T на пути U -> V, при этом (U; T) e E
struct FloidPathInfo {
    int r, subset, next_vertice;
    FloidPathInfo(int r=0, int subset=0, int next_vertice=0): r(r), subset(subset), next_vertice(next_vertice) {}
    FloidPathInfo operator&(const FloidPathInfo &other) {
        return FloidPathInfo(
            r + other.r,
            subset | other.subset,
            next_vertice
        );
    }
} floid[N][N]; // O(N^2)


// tsp[sparse_subset][U] - минимальный путь с концом в вершине U,
//  sparse_subset - ключевые вершины на пути
//  r - длина пути
//  full_subset - полное множество вершин на пути
//  prev_subset - предыдущее множество (для восстановления)
//  prev_vertice - предыдущая вершина (для восстановления)
struct TSPPathInfo {
    int r; // длина пути
    int full_subset; // полное множество вершин на пути
    int prev_subset, prev_vertice; // для восстановления маршрута
    TSPPathInfo(int r=0, int full_subset=0, int prev_subset=0, int prev_vertice=-1)
        : r(r)
        , full_subset(full_subset)
        , prev_subset(prev_subset)
        , prev_vertice(prev_vertice)
    {
    }
} tsp[1<<N][N]; // O(2^N * N)


// answer[subset] - минимальная суммарная стоимость всех операций,
//  чтобы получить поддерево с множеством вершин subset
struct Answer {
    int cost; // Стоимость, чтобы получить из исходного дерева текущее
    int prev_subset; // Предыдущее множество (для восстановления)
    int prev_path_subset; // Множество вершин, лежащим на пути в предыдущем раунде (для восстановления)
    int prev_path_vertice; // Конец пути в предыдущем раунде (для восстановления)
    Answer(int cost = 0, int prev_subset = 0, int prev_path_subset = 0, int prev_path_vertice = -1)
        : cost(cost)
        , prev_subset(prev_subset)
        , prev_path_subset(prev_path_subset)
        , prev_path_vertice(prev_path_vertice)
    {
    }
} answer[1<<N]; // O(2^N)


vector<int> neighbours[N]; // O(N) - список ребер
bool connected[N][N]; // O(N^2) - матрица смежности

// Операции для работы с множествами
inline int GetUniversalSubset(int n) {
    return (1 << n) - 1;
}

// Возвращает размер множества за O(1)
inline int GetSize(int subset) {
    subset = (subset >> 1 & 0x55555555) + (subset & 0x55555555);
    subset = (subset >> 2 & 0x33333333) + (subset & 0x33333333);
    subset = (subset >> 4 & 0x0F0F0F0F) + (subset & 0x0F0F0F0F);
    subset = (subset >> 8 & 0x00FF00FF) + (subset & 0x00FF00FF);
    subset += subset >> 16;
    return subset & 0xFFFF;
}

inline int MakeSubset(int vertice) {
    return 1 << vertice;
}

inline bool In(int vertice, int subset) {
    return MakeSubset(vertice) & subset;
}

inline int Add(int vertice, int subset) {
    return MakeSubset(vertice) | subset;
}

inline vector<int> GetElements(int subset) {
    vector<int> elements;
    for (int i = 0; i < N; i++) if (In(i, subset)) {
        elements.push_back(i);
    }
    return elements;
}


// n - количество вершин
// subset - множество вершин
// Работает за O(N)
bool IsSubGraphConnected(int n, int subset) {
    int count_vertices = 0;
    int first_vertice = -1;
    stack<int> dfs_stack;

    for (int i = 0; i < n; i++) {
        if (In(i, subset)) {
            count_vertices++, first_vertice = i;
        }
    }

    if (count_vertices == 0) return true;
    
    dfs_stack.push(first_vertice);
    int used = 0;
    while (!dfs_stack.empty()) {
        int u = dfs_stack.top();
        dfs_stack.pop();

        if (In(u, used)) continue;
        used = Add(u, used);

        for (int v : neighbours[u]) if (In(v, subset)) {
            dfs_stack.push(v);
        }
    }

    return used == subset;
}

// Возвращает подграф, состоящий из вершин subset + вершин, находящихся на расстоянии 1 от существующих вершин
// Асимптотика: O(N^2)
int ExtendSubraph(int n, int subset) {
    int extended_subset = subset;
    for (int u = 0; u < n; u++) if (In(u, subset)) {
        for (int v : neighbours[u]) {
            extended_subset = Add(v, extended_subset);
        }
    }
    return extended_subset;
}

int main() {
    int n; cin >> n; // количество вершин
    for (int i = 0; i < n-1; i++) { // считываем все ребра
        int u; cin >> u; u--;
        int v; cin >> v; v--;
        neighbours[u].push_back(v);
        neighbours[v].push_back(u);
        connected[u][v] = connected[v][u] = 1;
    }

    // === алгоритм Флойда ===
    // База
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (connected[i][j]) {
                floid[i][j].r = 1;
                floid[i][j].subset = MakeSubset(i) | MakeSubset(j);
                floid[i][j].next_vertice = j;
            } else if (i == j) {
                floid[i][j].r = 0;
                floid[i][j].subset = MakeSubset(i);
                floid[i][j].next_vertice = -1;
            } else {
                floid[i][j].r = INF;
            }
        }
    }

    // Основной цикл
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) if (i != k) {
            for (int j = 0; j < n; j++) if (j != i && j != k) {
                if (floid[i][k].r + floid[k][j].r < floid[i][j].r) {
                    floid[i][j] = floid[i][k] & floid[k][j];
                }
            }
        }
    }

    // Динамика TSP
    // База
    for (int mask = 0; mask <= GetUniversalSubset(n); mask++) {
        for (int u = 0; u < n; u++) if (In(u, mask)) {
            tsp[mask][u].r = INF;
        }
    }
    
    for (int u = 0; u < n; u++) {
        tsp[MakeSubset(u)][u] = TSPPathInfo(0, 0, -1);
    }

    // Алгоритм
    for (int mask = 0; mask < GetUniversalSubset(n); mask++) {
        for (int u = 0; u < n; u++) if (In(u, mask)) {
            for (int v = 0; v < n; v++) if (!In(v, mask)) {
                assert(floid[u][v].r != INF);

                if (tsp[mask | MakeSubset(v)][v].r > tsp[mask][u].r + floid[u][v].r) {
                    tsp[mask | MakeSubset(v)][v] = TSPPathInfo(
                        tsp[mask][u].r + floid[u][v].r,
                        tsp[mask][u].full_subset | floid[u][v].subset,
                        mask,
                        u
                    );
                }
            }
        }
    }


    // === Считаем ответ на задачу ===

    // база
    for (int tree = 0; tree < GetUniversalSubset(n); tree++) {
        answer[tree] = Answer(INF, 0, -1);
    }

    answer[GetUniversalSubset(n)] = Answer(0, 0, -1);

    // перебираем все поддеревья по невозрастанию его размера
    for (int tree = GetUniversalSubset(n); tree > 0; tree--) { // сколько вершин у нас осталось

        // Проверяем граф на связность
        if (!IsSubGraphConnected(n, tree)) {
            // Оставшееся поддерево не может быть не связным -> попасть в него не можем -> переходы из него невозможны
            continue;
        }
        assert(answer[tree].cost != INF);

        // Перебираем пути
        // На самом деле перебираем состояния TSP, который в свою очередь уже содержит путь
        for (int subtree = (tree - 1) & tree; subtree > 0; subtree = (subtree - 1) & tree) {
            // Если поддерево не связное или есть вершина, которая находится на расстоянии 2 и более, то скип
            if (!IsSubGraphConnected(n, subtree)) continue;
            if ((ExtendSubraph(n, subtree) & tree) != tree) continue;

            for (int u = 0; u < n; u++) if (In(u, subtree)) {
                if (tsp[subtree][u].r == INF) continue;
                int cost = 3 * tsp[subtree][u].r + 2 * GetSize(tree ^ subtree) + 1;

                // Переходы
                for (int v : neighbours[u]) if (In(v, tree) && !In(v, tsp[subtree][u].full_subset)) {
                    // Мы можем удалить вершину v, поскольку она не встречалась в пути -> черная,
                    //  а также находится в текущем поддереве tree
                    if (answer[tree ^ MakeSubset(v)].cost > answer[tree].cost + cost) {
                        answer[tree ^ MakeSubset(v)] = Answer(
                            answer[tree].cost + cost,
                            tree,
                            subtree,
                            u
                        );
                    }
                }
            }
        }
    }

    // === Восстановление ответа ===
    
    // находим вершину с самым наименьшим ответом
    vector<pair<int,int>> results;
    for (int u = 0; u < n; u++) {
        results.push_back({answer[MakeSubset(u)].cost, u});
    }
    int min_answer_vertice = min_element(results.begin(), results.end())->second;
    cout << "Optimal cost: " << answer[MakeSubset(min_answer_vertice)].cost << endl << endl;

    // выводить нужно наоборот, поэтому сохраняется все в вектор, который потом реверснем
    vector<stringstream> outputs;

    // идем по раундам с конца
    int current_tree = MakeSubset(min_answer_vertice);
    while (current_tree != GetUniversalSubset(n)) {
        assert(answer[current_tree].prev_path_vertice != -1);
        int prev_tree = answer[current_tree].prev_subset;

        assert(GetSize(current_tree ^ prev_tree) == 1);
        int missing_vertice = GetElements(current_tree ^ prev_tree)[0];

        int path_subset = answer[current_tree].prev_path_subset;
        int path_vertice = answer[current_tree].prev_path_vertice;

        // Считаем путь по ключевым точкам (каждая вершина встречается макс. 1 раз)
        // Это не совсем полный путь, а скорее просто основные ключевые точки
        vector<int> sparse_path(1, path_vertice);
        while (tsp[path_subset][path_vertice].prev_vertice != -1) {
            int prev_path_vertice = tsp[path_subset][path_vertice].prev_vertice;
            int prev_path_subset = tsp[path_subset][path_vertice].prev_subset;
            path_vertice = prev_path_vertice;
            path_subset = prev_path_subset;
            sparse_path.push_back(path_vertice);
        }
        reverse(sparse_path.begin(), sparse_path.end());

        // Полный путь (тут одна вершина может быть несколько раз)
        vector<int> full_path(1, sparse_path[0]);
        for (size_t i = 1; i < sparse_path.size(); i++) {
            int u = sparse_path[i-1];
            int v = sparse_path[i];
            while (u != v) {
                u = floid[u][v].next_vertice;
                full_path.push_back(u);
            }
        }

        // Вывод
        outputs.emplace_back();
        outputs.back() << "Remove vertice " << missing_vertice + 1 << endl;
        
        // путь по ключевым точкам
        outputs.back() << "Sparse Path: ";
        for (int u : sparse_path) outputs.back() << u + 1 << ' ';
        outputs.back() << endl;

        // полный путь
        outputs.back() << "Full path: ";
        for (int u : full_path) outputs.back() << u + 1 << ' ';
        outputs.back() << endl;

        // -- конец вывода

        current_tree = prev_tree;
    }

    reverse(outputs.begin(), outputs.end());
    for (size_t i = 0; i < outputs.size(); i++) {
        cout << "Round #" << i + 1 << endl;
        cout << outputs[i].str() << endl;
    }
}
