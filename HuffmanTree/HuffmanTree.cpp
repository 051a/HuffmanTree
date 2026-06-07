/*
 * 哈夫曼树应用程序 - 数据结构与算法课程设计
 *
 * 功能：
 *   1. 根据给定的字符频率创建哈夫曼树
 *   2. 打印哈夫曼树结构
 *   3. 打印所有字符及其哈夫曼编码
 *   4. (选做) 计算 "THIS PROGRAM IS MY FAVORITE" 的总报文传输长度
 *   5. (选做) 对用户输入的01串进行解码
 *
 * 构建规则：
 *   左孩子权值 <= 右孩子权值
 *   权值相同时，原序列中先出现的作为左孩子
 */

#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <cstdlib>

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

#define MAXNODE 100      // 最大结点数 (27叶子 + 26内部 = 53)
#define MAXLEN  200      // 最大编码长度

// ============ 哈夫曼树结点 ============
typedef struct {
    char  ch;            // 字符 (叶子结点有效)
    int   weight;        // 权值
    int   parent;        // 双亲下标, -1表示无
    int   lchild;        // 左孩子下标, -1表示无
    int   rchild;        // 右孩子下标, -1表示无
    int   order;         // 在原序列中的出现次序 (权值相同时判定左右)
} HTNode;

// ============ 哈夫曼编码表项 ============
typedef struct {
    char ch;             // 字符
    char bits[MAXLEN];   // 编码01串
    int  len;            // 编码长度
} HTCode;

// ============ 全局数据 ============
HTNode ht[MAXNODE];      // 哈夫曼树 (数组存储)
HTCode hc[27];           // 27个字符的编码
int    n = 27;           // 叶子结点数
int    m;                // 总结点数 = 2*n - 1 = 53

// 字符集: 空格 + 26个大写英文字母
char chars[27] = {
    ' ', 'A','B','C','D','E','F','G','H','I',
    'J','K','L','M','N','O','P','Q','R','S',
    'T','U','V','W','X','Y','Z'
};

// 对应频率
int weights[27] = {
    186, 64, 13, 22, 32, 103, 21, 15,
     47, 57,  1,  5, 32,  20, 57, 63,
     15,  1, 48, 51, 80,  23,  8, 18,
      1, 16,  1
};

// ============ 函数声明 ============
void InitHT();
void CreateHT();
void Select(int range, int &s1, int &s2);
void CreateHTCode();
void PrintHT();
void PrintCode();
void CalcTransLength(const char *str);
void DecodeInput();
void PrintMenu();

// ============ 主函数 ============
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);  /* 控制台输出UTF-8 (VS/Dev-C++均生效) */
    SetConsoleCP(65001);        /* 控制台输入UTF-8 */
#endif

    cout << endl;
    cout << "================================================" << endl;
    cout << "      哈夫曼树应用程序 (Huffman Tree)" << endl;
    cout << "      数据结构与算法课程设计" << endl;
    cout << "================================================" << endl;

    // 1. 创建哈夫曼树
    CreateHT();

    // 2. 生成哈夫曼编码
    CreateHTCode();

    // 3. 打印哈夫曼树结构
    PrintHT();

    // 4. 打印字符及其哈夫曼编码表
    PrintCode();

    // 5. 交互菜单
    int choice;
    do {
        PrintMenu();
        cout << "请输入选择 (0-2): ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1:
                CalcTransLength("THIS PROGRAM IS MY FAVORITE");
                break;
            case 2:
                DecodeInput();
                break;
            case 0:
                cout << endl << "程序结束，谢谢使用!" << endl;
                break;
            default:
                cout << "无效选择，请重新输入!" << endl;
        }
    } while (choice != 0);

    system("pause");  /* VS运行后暂停 */
    return 0;
}

// ============ 初始化哈夫曼树 ============
void InitHT() {
    m = 2 * n - 1;   // 53

    // 初始化叶子结点 (0 ~ 26)
    for (int i = 0; i < n; i++) {
        ht[i].ch     = chars[i];
        ht[i].weight = weights[i];
        ht[i].parent = -1;
        ht[i].lchild = -1;
        ht[i].rchild = -1;
        ht[i].order  = i;
    }

    // 初始化内部结点 (27 ~ 52)
    for (int i = n; i < m; i++) {
        ht[i].ch     = '#';
        ht[i].weight = 0;
        ht[i].parent = -1;
        ht[i].lchild = -1;
        ht[i].rchild = -1;
        ht[i].order  = i;
    }
}

// ============ 选择两个权值最小的根结点 ============
// 在 [0, range) 中选 parent==-1 的结点, 按 (权值, order) 比较
// s1: 最小的, s2: 次小的
void Select(int range, int &s1, int &s2) {
    s1 = -1;
    s2 = -1;

    for (int i = 0; i < range; i++) {
        if (ht[i].parent != -1) continue;

        if (s1 == -1) {
            s1 = i;
        } else if (ht[i].weight < ht[s1].weight ||
                  (ht[i].weight == ht[s1].weight && ht[i].order < ht[s1].order)) {
            // i 比 s1 更小
            s2 = s1;
            s1 = i;
        } else if (s2 == -1) {
            s2 = i;
        } else if (ht[i].weight < ht[s2].weight ||
                  (ht[i].weight == ht[s2].weight && ht[i].order < ht[s2].order)) {
            s2 = i;
        }
    }
}

// ============ 创建哈夫曼树 ============
void CreateHT() {
    InitHT();

    int s1, s2;
    for (int i = n; i < m; i++) {
        Select(i, s1, s2);

        ht[s1].parent = i;
        ht[s2].parent = i;

        ht[i].lchild = s1;   // 较小的在左
        ht[i].rchild = s2;   // 较大的在右
        ht[i].weight = ht[s1].weight + ht[s2].weight;
    }
}

// ============ 生成哈夫曼编码 (从叶子向根回溯) ============
void CreateHTCode() {
    for (int i = 0; i < n; i++) {
        hc[i].ch = chars[i];

        char temp[MAXLEN];
        int  pos  = 0;
        int  cur  = i;
        int  par  = ht[cur].parent;

        while (par != -1) {
            if (ht[par].lchild == cur) {
                temp[pos++] = '0';
            } else {
                temp[pos++] = '1';
            }
            cur = par;
            par = ht[cur].parent;
        }

        // 反转
        hc[i].len = pos;
        for (int j = 0; j < pos; j++) {
            hc[i].bits[j] = temp[pos - 1 - j];
        }
        hc[i].bits[pos] = '\0';
    }
}

// ============ 打印哈夫曼树结构 (横向缩进) ============
void PrintTreeRec(int idx, int depth) {
    if (idx == -1) return;

    // 先打印右子树 (显示在上方)
    PrintTreeRec(ht[idx].rchild, depth + 1);

    // 缩进
    for (int i = 0; i < depth; i++) {
        cout << "        ";
    }

    // 显示结点
    if (idx < n) {
        // 叶子结点
        char c = ht[idx].ch;
        cout << "(" << idx << ") ";
        if (c == ' ')
            cout << "<SP>:";
        else
            cout << c << ":";
        cout << ht[idx].weight;
        // 显示编码 (hc与ht叶子下标一一对应)
        cout << "  [" << hc[idx].bits << "]";

    } else {
        // 内部结点
        cout << "(" << idx << ") [" << ht[idx].weight << "]";
    }
    cout << endl;

    // 再打印左子树 (显示在下方)
    PrintTreeRec(ht[idx].lchild, depth + 1);
}

void PrintHT() {
    cout << endl;
    cout << "================================================" << endl;
    cout << "       哈夫曼树结构 (横向缩进显示)" << endl;
    cout << "  说明: 从上到下是根->叶子的方向" << endl;
    cout << "        叶子: (下标) 字符:权值 [编码]" << endl;
    cout << "        内部: (下标) [权值]" << endl;
    cout << "================================================" << endl;
    cout << endl;

    PrintTreeRec(m - 1, 0);

    cout << endl;
    cout << "================================================" << endl;
}

// ============ 打印字符及哈夫曼编码 ============
void PrintCode() {
    cout << endl;
    cout << "================================================" << endl;
    cout << "          字符及其哈夫曼编码表" << endl;
    cout << "================================================" << endl;
    cout << left;
    cout << setw(10) << "字符"
         << setw(10) << "权值"
         << setw(20) << "哈夫曼编码"
         << "编码长度" << endl;
    cout << string(55, '-') << endl;

    for (int i = 0; i < n; i++) {
        cout << setw(10);
        if (hc[i].ch == ' ')
            cout << "<SP>";
        else
            cout << hc[i].ch;

        cout << setw(10) << ht[i].weight;
        cout << setw(20) << hc[i].bits;
        cout << hc[i].len << endl;
    }
    cout << string(55, '-') << endl;
    cout << "共 " << n << " 个字符" << endl;
    cout << "================================================" << endl;
}

// ============ 计算指定字符串的传输长度 ============
void CalcTransLength(const char *str) {
    // 建立字符 -> 编码下标 的快速映射
    int map[256];
    for (int i = 0; i < 256; i++) map[i] = -1;
    for (int i = 0; i < n; i++) {
        map[(unsigned char)hc[i].ch] = i;
    }

    cout << endl;
    cout << "================================================" << endl;
    cout << "  计算报文传输长度" << endl;
    cout << "  原文: " << str << endl;
    cout << "================================================" << endl;
    cout << left;

    int totalBits = 0;
    int slen = (int)strlen(str);

    cout << endl << "逐字符编码:" << endl;
    cout << setw(8) << "字符"
         << setw(22) << "编码"
         << "位数" << endl;
    cout << string(42, '-') << endl;

    for (int i = 0; i < slen; i++) {
        char c = str[i];
        int  idx = map[(unsigned char)c];

        if (idx == -1) {
            cout << setw(8) << c << " (不在字符集中，跳过)" << endl;
            continue;
        }

        cout << setw(8);
        if (c == ' ') cout << "<SP>"; else cout << c;
        cout << setw(22) << hc[idx].bits;
        cout << hc[idx].len << endl;

        totalBits += hc[idx].len;
    }

    cout << string(42, '-') << endl;
    cout << "总报文传输长度: " << totalBits << " bits" << endl;
    cout << "================================================" << endl;
}

// ============ 解码用户输入的01串 ============
void DecodeInput() {
    string input;
    cout << endl;
    cout << "================================================" << endl;
    cout << "           哈夫曼解码" << endl;
    cout << "================================================" << endl;
    cout << "请输入01编码串: ";
    getline(cin, input);

    if (input.empty()) {
        cout << "输入为空!" << endl;
        cout << "================================================" << endl;
        return;
    }

    // 校验输入
    for (size_t i = 0; i < input.length(); i++) {
        if (input[i] != '0' && input[i] != '1') {
            cout << "解码失败 - 输入包含非0/1字符!" << endl;
            cout << "================================================" << endl;
            return;
        }
    }

    // 从根开始解码
    int    root    = m - 1;
    string result;
    int    current = root;

    for (size_t i = 0; i < input.length(); i++) {
        if (input[i] == '0') {
            current = ht[current].lchild;
        } else {
            current = ht[current].rchild;
        }

        if (current == -1) {
            cout << "解码失败 - 遇到无效编码路径!" << endl;
            cout << "================================================" << endl;
            return;
        }

        if (current < n) {
            // 到达叶子结点，输出字符
            result += ht[current].ch;
            current = root;
        }
    }

    // 结束后必须回到根
    if (current != root) {
        cout << "解码失败 - 编码串不完整，剩余位不足以构成完整字符!" << endl;
        cout << "================================================" << endl;
        return;
    }

    cout << endl << "解码结果: " << result << endl;
    cout << "================================================" << endl;
}

// ============ 功能菜单 ============
void PrintMenu() {
    cout << endl;
    cout << "+----------------------------------------------+" << endl;
    cout << "|                 功 能 菜 单                   |" << endl;
    cout << "+----------------------------------------------+" << endl;
    cout << "|  1. 计算报文传输长度                          |" << endl;
    cout << "|     (\"THIS PROGRAM IS MY FAVORITE\")          |" << endl;
    cout << "|  2. 解码01编码串                              |" << endl;
    cout << "|  0. 退出程序                                  |" << endl;
    cout << "+----------------------------------------------+" << endl;
}
