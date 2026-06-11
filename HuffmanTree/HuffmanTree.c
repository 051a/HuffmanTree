/*
 * 哈夫曼树应用程序 (C语言版) - 数据结构与算法课程设计
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
 *
 * 编码说明 (防止乱码)：
 *   本文件已转为 GBK 编码，Dev-C++ 直接打开编译即可，不显示乱码
 *   VS 中使用需重新保存为 UTF-8 编码
 */

#define _CRT_SECURE_NO_WARNINGS   /* VS安全函数警告抑制 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXNODE 100      /* 最大结点数 (27叶子 + 26内部 = 53) */
#define MAXLEN  200      /* 最大编码长度 */

/* ============ 哈夫曼树结点 ============ */
typedef struct {
    char  ch;            /* 字符 (叶子结点有效) */
    int   weight;        /* 权值 */
    int   parent;        /* 双亲下标, -1表示无 */
    int   lchild;        /* 左孩子下标, -1表示无 */
    int   rchild;        /* 右孩子下标, -1表示无 */
    int   order;         /* 在原序列中的出现次序 (权值相同时判定左右) */
} HTNode;

/* ============ 哈夫曼编码表项 ============ */
typedef struct {
    char ch;             /* 字符 */
    char bits[MAXLEN];   /* 编码01串 */
    int  len;            /* 编码长度 */
} HTCode;

/* ============ 全局数据 ============ */
HTNode ht[MAXNODE];      /* 哈夫曼树 (数组存储) */
HTCode hc[27];           /* 27个字符的编码 */
int    n = 27;           /* 叶子结点数 */
int    m;                /* 总结点数 = 2*n - 1 = 53 */

/* 字符集: 空格 + 26个大写英文字母 */
char chars[27] = {
    ' ', 'A','B','C','D','E','F','G','H','I',
    'J','K','L','M','N','O','P','Q','R','S',
    'T','U','V','W','X','Y','Z'
};

/* 对应频率 */
int weights[27] = {
    186, 64, 13, 22, 32, 103, 21, 15,
     47, 57,  1,  5, 32,  20, 57, 63,
     15,  1, 48, 51, 80,  23,  8, 18,
      1, 16,  1
};

/* ============ 函数声明 ============ */
void InitHT(void);
void CreateHT(void);
void Select(int range, int *s1, int *s2);
void CreateHTCode(void);
void PrintHT(void);
void PrintCode(void);
void CalcTransLength(const char *str);
void DecodeInput(void);
void PrintMenu(void);

/* 递归打印树的辅助函数 */
void PrintTreeRec(int idx, int depth);

/* ============ 主函数 ============ */
int main(void) {
    int choice;

    printf("\n");
    printf("================================================\n");
    printf("      哈夫曼树应用程序 (Huffman Tree)\n");
    printf("      数据结构与算法课程设计\n");
    printf("================================================\n");

    /* 1. 创建哈夫曼树 */
    CreateHT();

    /* 2. 生成哈夫曼编码 */
    CreateHTCode();

    /* 3. 打印哈夫曼树结构 */
    PrintHT();

    /* 4. 打印字符及其哈夫曼编码表 */
    PrintCode();

    /* 5. 交互菜单 */
    do {
        PrintMenu();
        printf("请输入选择 (0-2): ");
        scanf("%d", &choice);
        getchar();  /* 清除缓冲区中的换行符 */

        switch (choice) {
            case 1:
                CalcTransLength("THIS PROGRAM IS MY FAVORITE");
                break;
            case 2:
                DecodeInput();
                break;
            case 0:
                printf("\n程序结束，谢谢使用!\n");
                break;
            default:
                printf("无效选择，请重新输入!\n");
        }
    } while (choice != 0);

    system("pause");  /* VS运行后暂停，防止控制台闪退 */
    return 0;
}

/* ============ 初始化哈夫曼树 ============ */
void InitHT(void) {
    int i;

    m = 2 * n - 1;  /* 53 */

    /* 初始化叶子结点 (0 ~ 26) */
    for (i = 0; i < n; i++) {
        ht[i].ch     = chars[i];
        ht[i].weight = weights[i];
        ht[i].parent = -1;
        ht[i].lchild = -1;
        ht[i].rchild = -1;
        ht[i].order  = i;
    }

    /* 初始化内部结点 (27 ~ 52) */
    for (i = n; i < m; i++) {
        ht[i].ch     = '#';
        ht[i].weight = 0;
        ht[i].parent = -1;
        ht[i].lchild = -1;
        ht[i].rchild = -1;
        ht[i].order  = i;
    }
}

/* ============ 选择两个权值最小的根结点 ============ */
/* 在 [0, range) 中选 parent==-1 的结点, 按 (权值, order) 比较 */
/* *s1: 最小的, *s2: 次小的 */
void Select(int range, int *s1, int *s2) {
    int i;

    *s1 = -1;
    *s2 = -1;

    for (i = 0; i < range; i++) {
        if (ht[i].parent != -1) continue;

        if (*s1 == -1) {
            *s1 = i;
        } else if (ht[i].weight < ht[*s1].weight ||
                  (ht[i].weight == ht[*s1].weight && ht[i].order < ht[*s1].order)) {
            /* i 比 *s1 更小 */
            *s2 = *s1;
            *s1 = i;
        } else if (*s2 == -1) {
            *s2 = i;
        } else if (ht[i].weight < ht[*s2].weight ||
                  (ht[i].weight == ht[*s2].weight && ht[i].order < ht[*s2].order)) {
            *s2 = i;
        }
    }
}

/* ============ 创建哈夫曼树 ============ */
void CreateHT(void) {
    int i;
    int s1, s2;

    InitHT();

    for (i = n; i < m; i++) {
        Select(i, &s1, &s2);

        ht[s1].parent = i;
        ht[s2].parent = i;

        ht[i].lchild = s1;   /* 较小的在左 */
        ht[i].rchild = s2;   /* 较大的在右 */
        ht[i].weight = ht[s1].weight + ht[s2].weight;
    }
}

/* ============ 生成哈夫曼编码 (从叶子向根回溯) ============ */
void CreateHTCode(void) {
    int i, j;
    int cur, par, pos;
    char temp[MAXLEN];

    for (i = 0; i < n; i++) {
        hc[i].ch = chars[i];

        pos = 0;
        cur = i;
        par = ht[cur].parent;

        while (par != -1) {
            if (ht[par].lchild == cur) {
                temp[pos++] = '0';
            } else {
                temp[pos++] = '1';
            }
            cur = par;
            par = ht[cur].parent;
        }

        /* 反转 */
        hc[i].len = pos;
        for (j = 0; j < pos; j++) {
            hc[i].bits[j] = temp[pos - 1 - j];
        }
        hc[i].bits[pos] = '\0';
    }
}

/* ============ 打印哈夫曼树结构 (横向缩进) ============ */
void PrintTreeRec(int idx, int depth) {
    int i;
    char c;

    if (idx == -1) return;

    /* 先打印右子树 (显示在上方) */
    PrintTreeRec(ht[idx].rchild, depth + 1);

    /* 缩进 */
    for (i = 0; i < depth; i++) {
        printf("        ");
    }

    /* 显示结点 */
    if (idx < n) {
        /* 叶子结点 */
        c = ht[idx].ch;
        printf("(%d) ", idx);
        if (c == ' ')
            printf("<SP>:");
        else
            printf("%c:", c);
        printf("%d", ht[idx].weight);
        /* 显示编码 (hc与ht叶子下标一一对应) */
        printf("  [%s]", hc[idx].bits);
    } else {
        /* 内部结点 */
        printf("(%d) [%d]", idx, ht[idx].weight);
    }
    printf("\n");

    /* 再打印左子树 (显示在下方) */
    PrintTreeRec(ht[idx].lchild, depth + 1);
}

void PrintHT(void) {
    printf("\n");
    printf("================================================\n");
    printf("       哈夫曼树结构 (横向缩进显示)\n");
    printf("  说明: 从上到下是根->叶子的方向\n");
    printf("        叶子: (下标) 字符:权值 [编码]\n");
    printf("        内部: (下标) [权值]\n");
    printf("================================================\n");
    printf("\n");

    PrintTreeRec(m - 1, 0);

    printf("\n");
    printf("================================================\n");
}

/* ============ 打印字符及哈夫曼编码 ============ */
void PrintCode(void) {
    int i;

    printf("\n");
    printf("================================================\n");
    printf("          字符及其哈夫曼编码表\n");
    printf("================================================\n");
    printf("%-10s%-10s%-20s%s\n", "字符", "权值", "哈夫曼编码", "编码长度");
    printf("-------------------------------------------------------\n");

    for (i = 0; i < n; i++) {
        if (hc[i].ch == ' ')
            printf("%-10s", "<SP>");
        else
            printf("%-10c", hc[i].ch);
        printf("%-10d", ht[i].weight);
        printf("%-20s", hc[i].bits);
        printf("%d\n", hc[i].len);
    }
    printf("-------------------------------------------------------\n");
    printf("共 %d 个字符\n", n);
    printf("================================================\n");
}

/* ============ 计算指定字符串的传输长度 ============ */
void CalcTransLength(const char *str) {
    int map[256];
    int i, idx, totalBits, slen;
    char c;

    /* 建立字符 -> 编码下标 的快速映射 */
    for (i = 0; i < 256; i++) map[i] = -1;
    for (i = 0; i < n; i++) {
        map[(unsigned char)hc[i].ch] = i;
    }

    printf("\n");
    printf("================================================\n");
    printf("  计算报文传输长度\n");
    printf("  原文: %s\n", str);
    printf("================================================\n");

    totalBits = 0;
    slen = (int)strlen(str);

    printf("\n逐字符编码:\n");
    printf("%-8s%-22s%s\n", "字符", "编码", "位数");
    printf("------------------------------------------\n");

    for (i = 0; i < slen; i++) {
        c = str[i];
        idx = map[(unsigned char)c];

        if (idx == -1) {
            printf("%-8c (不在字符集中，跳过)\n", c);
            continue;
        }

        if (c == ' ')
            printf("%-8s", "<SP>");
        else
            printf("%-8c", c);
        printf("%-22s", hc[idx].bits);
        printf("%d\n", hc[idx].len);

        totalBits += hc[idx].len;
    }

    printf("------------------------------------------\n");
    printf("总报文传输长度: %d bits\n", totalBits);
    printf("================================================\n");
}

/* ============ 解码用户输入的01串 ============ */
void DecodeInput(void) {
    char input[MAXLEN];
    char result[MAXLEN];
    int  i, len, root, current, resPos;
    int  valid;

    printf("\n");
    printf("================================================\n");
    printf("           哈夫曼解码\n");
    printf("================================================\n");
    printf("请输入01编码串: ");
    fgets(input, MAXLEN, stdin);

    /* 去掉 fgets 读入的换行符 */
    len = (int)strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
        len--;
    }

    if (len == 0) {
        printf("输入为空!\n");
        printf("================================================\n");
        return;
    }

    /* 校验输入 */
    valid = 1;
    for (i = 0; i < len; i++) {
        if (input[i] != '0' && input[i] != '1') {
            valid = 0;
            break;
        }
    }
    if (!valid) {
        printf("解码失败 - 输入包含非0/1字符!\n");
        printf("================================================\n");
        return;
    }

    /* 从根开始解码 */
    root    = m - 1;
    current = root;
    resPos  = 0;
    result[0] = '\0';

    for (i = 0; i < len; i++) {
        if (input[i] == '0') {
            current = ht[current].lchild;
        } else {
            current = ht[current].rchild;
        }

        if (current == -1) {
            printf("解码失败 - 遇到无效编码路径!\n");
            printf("================================================\n");
            return;
        }

        if (current < n) {
            /* 到达叶子结点，输出字符 */
            result[resPos++] = ht[current].ch;
            result[resPos] = '\0';
            current = root;
        }
    }

    /* 结束后必须回到根 */
    if (current != root) {
        printf("解码失败 - 编码串不完整，剩余位不足以构成完整字符!\n");
        printf("================================================\n");
        return;
    }

    printf("\n解码结果: %s\n", result);
    printf("================================================\n");
}

/* ============ 功能菜单 ============ */
void PrintMenu(void) {
    printf("\n");
    printf("+----------------------------------------------+\n");
    printf("|                 功 能 菜 单                   |\n");
    printf("+----------------------------------------------+\n");
    printf("|  1. 计算报文传输长度                          |\n");
    printf("|     (\"THIS PROGRAM IS MY FAVORITE\")          |\n");
    printf("|  2. 解码01编码串                              |\n");
    printf("|  0. 退出程序                                  |\n");
    printf("+----------------------------------------------+\n");
}
