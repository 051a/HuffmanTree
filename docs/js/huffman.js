/**
 * 哈夫曼树 (Huffman Tree) - JavaScript 实现
 * 移植自 HuffmanTree.c，保持算法逻辑完全一致
 */

/* ============ 哈夫曼树结点 ============ */
class HTNode {
    constructor(ch, weight, order) {
        this.ch     = ch;      // 字符 (叶子结点有效)
        this.weight = weight;  // 权值
        this.parent = -1;      // 双亲下标, -1表示无
        this.lchild = -1;      // 左孩子下标, -1表示无
        this.rchild = -1;      // 右孩子下标, -1表示无
        this.order  = order;   // 在原序列中的出现次序 (权值相同时判定左右)
    }
}

/* ============ 哈夫曼编码表项 ============ */
class HTCode {
    constructor(ch, bits, len) {
        this.ch   = ch;    // 字符
        this.bits = bits;  // 编码01串
        this.len  = len;   // 编码长度
    }
}

/* ============ 哈夫曼树主类 ============ */
class HuffmanTree {
    constructor() {
        // 字符集: 空格 + 26个大写英文字母
        this.chars = [
            ' ', 'A','B','C','D','E','F','G','H','I',
            'J','K','L','M','N','O','P','Q','R','S',
            'T','U','V','W','X','Y','Z'
        ];

        // 对应频率
        this.weights = [
            186, 64, 13, 22, 32, 103, 21, 15,
             47, 57,  1,  5, 32,  20, 57, 63,
             15,  1, 48, 51, 80,  23,  8, 18,
              1, 16,  1
        ];

        this.n = 27;            // 叶子结点数
        this.m = 2 * this.n - 1; // 总结点数 = 53
        this.nodes = [];        // 哈夫曼树结点数组
        this.codes = [];        // 哈夫曼编码表

        // 构建哈夫曼树
        this.initNodes();
        this.buildTree();
        this.buildCodes();
    }

    /* ============ 初始化哈夫曼树结点 ============ */
    initNodes() {
        // 初始化叶子结点 (0 ~ 26)
        for (let i = 0; i < this.n; i++) {
            this.nodes.push(new HTNode(this.chars[i], this.weights[i], i));
        }
        // 初始化内部结点 (27 ~ 52)
        for (let i = this.n; i < this.m; i++) {
            this.nodes.push(new HTNode('#', 0, i));
        }
    }

    /* ============ 选择两个权值最小的根结点 ============ */
    /* 在 [0, range) 中选 parent==-1 的结点, 按 (权值, order) 比较 */
    /* 返回 [s1, s2]: s1是最小的, s2是次小的 */
    select(range) {
        let s1 = -1;
        let s2 = -1;

        for (let i = 0; i < range; i++) {
            if (this.nodes[i].parent !== -1) continue;

            if (s1 === -1) {
                s1 = i;
            } else if (this.nodes[i].weight < this.nodes[s1].weight ||
                      (this.nodes[i].weight === this.nodes[s1].weight &&
                       this.nodes[i].order < this.nodes[s1].order)) {
                // i 比 s1 更小
                s2 = s1;
                s1 = i;
            } else if (s2 === -1) {
                s2 = i;
            } else if (this.nodes[i].weight < this.nodes[s2].weight ||
                      (this.nodes[i].weight === this.nodes[s2].weight &&
                       this.nodes[i].order < this.nodes[s2].order)) {
                s2 = i;
            }
        }

        return [s1, s2];
    }

    /* ============ 创建哈夫曼树 ============ */
    buildTree() {
        for (let i = this.n; i < this.m; i++) {
            const [s1, s2] = this.select(i);

            this.nodes[s1].parent = i;
            this.nodes[s2].parent = i;

            this.nodes[i].lchild = s1;  // 较小的在左
            this.nodes[i].rchild = s2;  // 较大的在右
            this.nodes[i].weight = this.nodes[s1].weight + this.nodes[s2].weight;
        }
    }

    /* ============ 生成哈夫曼编码 (从叶子向根回溯) ============ */
    buildCodes() {
        for (let i = 0; i < this.n; i++) {
            let temp = [];
            let cur = i;
            let par = this.nodes[cur].parent;

            while (par !== -1) {
                if (this.nodes[par].lchild === cur) {
                    temp.push('0');
                } else {
                    temp.push('1');
                }
                cur = par;
                par = this.nodes[cur].parent;
            }

            // 反转
            temp.reverse();
            const bits = temp.join('');
            this.codes.push(new HTCode(this.chars[i], bits, bits.length));
        }
    }

    /* ============ 生成横向缩进文本树 (递归) ============ */
    _getTreeTextRec(idx, depth, lines) {
        if (idx === -1) return;

        // 先打印右子树 (显示在上方)
        this._getTreeTextRec(this.nodes[idx].rchild, depth + 1, lines);

        // 缩进
        let indent = '';
        for (let i = 0; i < depth; i++) {
            indent += '        ';
        }

        // 结点信息
        let line = indent;
        if (idx < this.n) {
            // 叶子结点
            const c = this.nodes[idx].ch;
            const displayCh = (c === ' ') ? '&lt;SP&gt;' : c;
            line += `(${idx}) ${displayCh}:${this.nodes[idx].weight}`;
            if (idx < this.codes.length) {
                line += `  [${this.codes[idx].bits}]`;
            }
        } else {
            // 内部结点
            line += `(${idx}) [${this.nodes[idx].weight}]`;
        }
        lines.push(line);

        // 再打印左子树 (显示在下方)
        this._getTreeTextRec(this.nodes[idx].lchild, depth + 1, lines);
    }

    getTreeText() {
        const lines = [];
        this._getTreeTextRec(this.m - 1, 0, lines);
        return lines;
    }

    /* ============ 获取编码表数据 ============ */
    getCodeTable() {
        const table = [];
        for (let i = 0; i < this.n; i++) {
            table.push({
                ch:    this.codes[i].ch,
                displayCh: (this.codes[i].ch === ' ') ? '<SP>' : this.codes[i].ch,
                weight: this.nodes[i].weight,
                bits:   this.codes[i].bits,
                len:    this.codes[i].len
            });
        }
        return table;
    }

    /* ============ 计算指定字符串的传输长度 ============ */
    calcTransLength(str) {
        // 建立字符 -> 编码下标 的快速映射
        const map = new Array(256).fill(-1);
        for (let i = 0; i < this.n; i++) {
            map[this.codes[i].ch.charCodeAt(0)] = i;
        }

        const details = [];
        let totalBits = 0;

        for (let i = 0; i < str.length; i++) {
            const c = str[i];
            const idx = map[c.charCodeAt(0)];

            if (idx === -1) {
                details.push({
                    ch: c,
                    displayCh: c,
                    bits: '(不在字符集中)',
                    len: 0,
                    valid: false
                });
                continue;
            }

            const displayCh = (c === ' ') ? '<SP>' : c;
            details.push({
                ch: c,
                displayCh: displayCh,
                bits: this.codes[idx].bits,
                len: this.codes[idx].len,
                valid: true
            });
            totalBits += this.codes[idx].len;
        }

        return { details, totalBits };
    }

    /* ============ 解码01串 ============ */
    decode(binaryStr) {
        // 去除空白字符
        const cleaned = binaryStr.replace(/\s/g, '');

        if (cleaned.length === 0) {
            return { success: false, error: '输入为空!', result: '' };
        }

        // 校验输入
        for (let i = 0; i < cleaned.length; i++) {
            if (cleaned[i] !== '0' && cleaned[i] !== '1') {
                return { success: false, error: '解码失败 - 输入包含非0/1字符!', result: '' };
            }
        }

        const root = this.m - 1;
        let current = root;
        let result = '';

        for (let i = 0; i < cleaned.length; i++) {
            if (cleaned[i] === '0') {
                current = this.nodes[current].lchild;
            } else {
                current = this.nodes[current].rchild;
            }

            if (current === -1) {
                return { success: false, error: '解码失败 - 遇到无效编码路径!', result: '' };
            }

            if (current < this.n) {
                // 到达叶子结点
                result += this.nodes[current].ch;
                current = root;
            }
        }

        // 结束后必须回到根
        if (current !== root) {
            return {
                success: false,
                error: '解码失败 - 编码串不完整，剩余位不足以构成完整字符!',
                result: ''
            };
        }

        return { success: true, error: '', result: result };
    }

    /* ============ 获取SVG树形图所需数据 ============ */
    /* 返回节点位置和连线信息 */
    /* 算法：中序遍历分配x坐标，DFS从根计算depth作为y坐标 */
    getTreeLayout() {
        const root = this.m - 1;

        // Step 1: DFS从根计算每个节点的深度 (根depth=0)
        const depthArr = new Array(this.m).fill(0);
        const dfsDepth = (idx, d) => {
            if (idx === -1) return;
            depthArr[idx] = d;
            dfsDepth(this.nodes[idx].lchild, d + 1);
            dfsDepth(this.nodes[idx].rchild, d + 1);
        };
        dfsDepth(root, 0);
        const maxDepth = Math.max(...depthArr);

        // Step 2: 中序遍历分配x坐标
        const positions = new Array(this.m);
        let inorderX = 0;

        const inorder = (idx) => {
            if (idx === -1) return;
            inorder(this.nodes[idx].lchild);
            positions[idx] = {
                x: inorderX,
                y: depthArr[idx],
                isLeaf: idx < this.n
            };
            inorderX++;
            inorder(this.nodes[idx].rchild);
        };
        inorder(root);

        // Step 3: 收集连线
        const edges = [];
        for (let i = 0; i < this.m; i++) {
            if (this.nodes[i].lchild !== -1) {
                edges.push({ from: i, to: this.nodes[i].lchild, side: '0' });
            }
            if (this.nodes[i].rchild !== -1) {
                edges.push({ from: i, to: this.nodes[i].rchild, side: '1' });
            }
        }

        return {
            positions,
            edges,
            nodeCount: inorderX,    // 总节点数 = 53
            totalDepth: maxDepth + 1,
            n: this.n,
            m: this.m
        };
    }
}
