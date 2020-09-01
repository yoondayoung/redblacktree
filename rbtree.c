#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// red black tree ����
typedef struct rbnode_ {
	int key;
	struct rbnode_* lchild; struct rbnode_* rchild;
	bool red; // true�� red node, �ƴϸ� black node
	unsigned long long stocks;// �ߺ� ��� ���� key � �ִ���
}rbnode;

//��� ������
rbnode* new_node(){
	rbnode* new = (rbnode*)malloc(sizeof(rbnode));
	new->lchild = new->rchild = NULL;
	new->red = false;
	new->stocks = 1;
	return new;
}

// linked stack
typedef struct snode_ {
	rbnode* element;
	struct snode_* link;
}snode;

void push(snode** top, rbnode* node) {
	snode* temp = (snode*)malloc(sizeof(snode));
	temp->element = node;
	temp->link = *top;
	*top = temp;
}

rbnode* pop(snode** top) {
	snode* temp = *top;
	rbnode* item;
	if (*top == NULL) {
		return NULL;
	}
	item = temp->element;
	*top = temp->link;
	free(temp);
	return item;
}

// rbt operations
bool search(rbnode* root, int key); // ã���� true ��ã���� false
void insert(rbnode* root, int key);
bool delete(rbnode* root, int key); // ����� true �ش� ������ �������� false

// rbt ���� �Լ�
rbnode* rotate(rbnode* root, int k, rbnode* pv );
bool addstocks(rbnode* root, int k);
void removenode(rbnode** node);
bool dellast(rbnode* root, int k, rbnode* dp, rbnode** dc);
bool redtoP(rbnode* dgp, rbnode* dp, rbnode* dc, rbnode *dsb);
void reddown(rbnode* dp);
bool isonlyblack(rbnode* node);
bool redrotate(rbnode* root, rbnode* dgp, rbnode* dp, rbnode*dc, rbnode* dsb);
int replaceK(rbnode *dc);
bool possibleD(rbnode* node);
bool delstocks(rbnode* root, int k);

// �޸� �Ҵ� ���� �Լ�
void clear(rbnode** root) {
	snode* stack = NULL;
	rbnode *current, *del;
	current = (*root)->lchild;
	for (;;) {
		for (; current; current = current->lchild)
			push(&stack, current);
		current = pop(&stack);
		if (!current) break;
		del = current;
		current = current->rchild;
		free(del);
	}
	free(*root);
	*root = NULL;
	return;
}

int main(int argc, char* argv[]) {
	clock_t start_time = clock();
	FILE *ifp, *ofp;
	rbnode* rbtroot;
	int res, k, i = 0;
	char func;

	if (argc != 2) {
		printf("please input 2 argv .\n");
	}
	ifp = fopen(argv[1], "r");
	if (ifp == NULL) {
		printf("input file open error.\n");
		return 1;
	}
	ofp = fopen("db_result.txt", "w");
	if (ofp == NULL) {
		printf("output file open error.\n");
		return 1;
	}

	rbtroot = new_node(); // red black tree�� root -> ����ְ� ��child���� ��
	rbtroot->rchild = rbtroot; rbtroot->key = 0; rbtroot->stocks = -1; // dummy node

	while (1) {
		res = fscanf(ifp, "%c %d", &func, &k);
		if (res == EOF)
			break; // ���о����� ����
		if (func == 'i') { // ����
			insert(rbtroot, k);
		}
		if (func == 'd') {
			if (delete(rbtroot, k) == false) 
				fprintf(ofp, "ignore\n");
		}
		if (func == 's') {
			if (search(rbtroot, k) == true)
				fprintf(ofp, "true\n");
			else fprintf(ofp, "false\n");
		}
	}
	clear(&rbtroot);
	fclose(ifp);
	fclose(ofp);
	printf("%lf\n", (double)(clock() - start_time) / CLOCKS_PER_SEC);
	return 0;
}

//bst�� Ž�� ��� ����
bool search(rbnode* root, int k) { 
	root = root->lchild;
	while (root) {
		if (k == root->key) return true;
		else if (k < root->key) root = root->lchild;
		else root = root->rchild;
	}
	return false;
}

rbnode* rotate(rbnode* root, int k, rbnode* pv) {
	// key -> ũ�� ������ roatate ���� ���� (gchild�� ��)
	rbnode *child, *gchild;
	// pv�� child ã���ֱ�
	if ((k > pv->key || k == pv->key) && pv != root)
		child = pv->rchild;
	else
		child = pv->lchild;
	// ���� ȸ��
	if (k > child->key || k == child->key) {
		gchild = child->rchild; // gchild ã���ֱ�
		child->rchild = gchild->lchild;
		gchild->lchild = child;
	}
	else {// ������ ȸ��
		gchild = child->lchild; // gchild ã���ֱ�
		child->lchild = gchild->rchild;
		gchild->rchild = child;
	}

	// pv�� ��� ������ ���Ѵ�.
	if ((k > pv->key || k == pv->key) && pv != root)
		pv->rchild = gchild;
	else
		pv->lchild = gchild;
	// ȸ�� �� pv�� �ڽĳ�� ��ȯ
	return gchild;
}

// stocks�� 2 �̻��̸� stocks�� ���� -> ���� �Ϸ�
bool addstocks(rbnode* root, int k) {
	root = root->lchild;
	while (root) {
		if (k == root->key) {
			root->stocks++;
			return true;
		}
		else if (k < root->key) root = root->lchild;
		else root = root->rchild;
	}
	return false;
}

void insert(rbnode* root, int k) {
	rbnode *c, *p, *gp, *ggp; // �ڽ�, �θ�, ���θ�, �����θ� ���� ����
	p = gp = ggp = root;
	c = root->lchild;
	if (addstocks(root, k) == true)
		return;
	else {
		while (c) {
			if (c->lchild && c->rchild && c->lchild->red == true && c->rchild->red == true) {
				// �ؿ� �� �ڽ��� red node -> �������ȯ
				c->red = true;
				c->lchild->red = c->rchild->red = false;
				if (p->red == true) {
					// red node ���޾� ������ -> ȸ���������
					gp->red = true; // ��� ��쿡�� gp�� red�� �ٲ�
					if ((k > gp->key) != (k > p->key))
						// ������ ���̴� ��쿡�� double rotate ����
						p = rotate(root, k, gp);
					//������ �Ȳ��̸� �׳� rotate �ѹ�
					c = rotate(root, k, ggp);
					c->red = false; // ��� ��쿡�� c�� black���� �ٲ�
				}
				// root�� �׻� black
				root->lchild->red = false;
			}
			// ���� ���� ����
			ggp = gp; gp = p; p = c;
			if (k > c->key) c = c->rchild;
			else c = c->lchild;
		}
		// �˸��� ��ġ�� leaf node ã��-> ��� ���� �����
		c = new_node();
		c->key = k;
		c->red = true; // ���� ���ԵǴ� ���� red
		if (k > p->key&&p != root)
			p->rchild = c;
		else
			p->lchild = c;
		if (p->red == true) {
			gp->red = true;
			if ((k > gp->key) != (k > p->key))
				p = rotate(root, k, gp);
			c = rotate(root, k, ggp);
			c->red = false;
		}
		// root�� �׻� ����
		root->lchild->red = false;
		return;
	}
}

void removenode(rbnode** node) {
	// �ڷᰡ 1�� ������ ���� �� �޸� ����
	rbnode* temp = *node;
	*node = NULL;
	free(temp);
	return;
}

bool dellast(rbnode* root, int k, rbnode* dp, rbnode** dc) { // ������� �ϴ� Ű, ��������ϴ� �θ���, ��������ϴ� ��� �ڽ�
	if (k == (*dc)->key && !((*dc)->lchild) && !((*dc)->rchild)) {
		// dc, ��������ϴ� ���� leaf ����� ���(red) -> black node�� ����� ���� (color demotion -> ���� ����ȯ)
		removenode(dc);
		// �θ����� ��ũ ���� ����
		if ((k > dp->key || k == dp->key) && dp != root) // ���γ���� ���� ��Ģ(�θ� key�� ���������� ����)
			// ���� ���� ������ subtree���� ���� ���� ��(inorder successor)�� ���� �÷ȱ� ������ Ű���� �θ� Ű���� ���� ��� ���������� �̵�
			dp->rchild = NULL;
		else dp->lchild = NULL;
		return true;
	}
	else if (k == (*dc)->key) {
		// dc, ��������ϴ� ���� black node�� ���(leaf node�� �θ�)
		rbnode* sub = NULL; // ���� ��ġ(dc)�� ��� �� ��
		if ((*dc)->lchild) {
			// lchild�� ������ lchild�� �θ� �ؿ� ���ϰ�
			(*dc)->lchild->rchild = (*dc)->rchild;
			sub = (*dc)->lchild;
			sub->red = false;
			removenode(dc);
		}
		else if ((*dc)->rchild) {
			(*dc)->rchild->lchild = (*dc)->lchild;
			sub = (*dc)->rchild;
			sub->red = false;
			removenode(dc);
		}
		if ((sub->key > dp->key || sub->key == dp->key) && dp != root)
			dp->rchild = sub;
		else dp->lchild = sub;
		return true;
	}
	// ����尡 dc�̰� ������� �ϴ� ���� dc�� �ڽĳ��(red node)�϶�
	else if ((*dc)->lchild && k == (*dc)->lchild->key) {
		removenode(&((*dc)->lchild));
		(*dc)->lchild = NULL;
		return true;
	}
	else if ((*dc)->rchild && k == (*dc)->rchild->key) {
		removenode(&((*dc)->rchild));
		(*dc)->rchild = NULL;
		return true;
	}
	else return false; // ���� ����
}

// dc-> ����� ������ �ִ� ������
// Ž�� ����� �θ��带 �׻� red node�� ���� ����
bool redtoP(rbnode* root, rbnode* dgp, rbnode* dp, rbnode* dsb) {
	if (dsb == NULL || dsb->red == false) // ������尡 ���ų� ������尡 black node�̸�
		return false;
	// dp, dsb rotate -> dsb�� dp�� �θ�, dc�� ���θ� ��
	rotate(root, dsb->key, dgp);
	dsb->red = false;
	// dc�� �θ��� dp�� red node�� ��
	dp->red = true;
	return true;
}

// (�θ�� ���� �پ��ִ� ������ �и���Ű��) �θ�, ������ ���� -> ���� ����ȯ
// ����: ���� ���� sibling�� black�̰�, �� �ڽİ����� �Ѵ� black => �ȱ׷��� �������� ����
void reddown(rbnode* dp) {
	// �θ�� black
	dp->red = false;
	// �ڽĵ��� red
	dp->lchild->red = dp->rchild->red = true;
	return;
}

// �ڽ�, �ڽ� ��尡 ��� black���� Ȯ��
bool isonlyblack(rbnode* node) {
	if (!node) return false;
	if (node->red == true) return false;
	if ((!(node->lchild) && !(node->rchild)) ||
		(node->lchild&&node->rchild && (node->lchild->red == false) && (node->rchild->red == false)))
		return true;
	else return false;
}

// red node ���� ���� => rotate�ؼ� �����带 red��
// ����: sibling: black, �ϳ��̻��� red �ڽ� ����
bool redrotate(rbnode* root, rbnode* dgp, rbnode* dp, rbnode* dc, rbnode* dsb) {
	rbnode *dsbrc; // dsb�� red �ڽĳ��
	if (isonlyblack(dsb)==true) return false; // red node ������ ���� ������
	if (dc->key > dsb->key) { // dsb�� ����, dc�� ������ �ڽĳ��
		if (dsb->lchild && dsb->lchild->red == true)
			// ���� �ڽ����� red node�� �������
			dsbrc = dsb->lchild;
		else dsbrc = dsb->rchild;
	}
	else { // dsb�� ������, dc�� ����
		if (dsb->rchild && dsb->rchild->red == true)
			// red node�� ������ �ڽ��� ���� ���
			dsbrc = dsb->rchild;
		else dsbrc = dsb->lchild;
	}
	if ((dp->key > dsb->key) == (dsb->key > dsbrc->key)) {
		// �߰��� �Ȳ��������� ȸ�� �ѹ�
		rotate(root, dsbrc->key, dgp);
		dsb->red = true;
		dsbrc->red = false;
	}
	else { // �������� ȸ�� �ι�
		rotate(root, dsbrc->key, dp);
		rotate(root, dsbrc->key, dgp);
		dsb->red = false;
		dsbrc->red = true;
	}
	dc->red = true;
	dp->red = false;
	// root�� �׻� black
	if (root->lchild->red == true)
		root->lchild->red = false;
	return true;
}

// �߰��� �ִ� �� ������ ���� ����
// �߰��� �ִ� ���� �� �� ����� ������ ��ü -> �� leaf��带 ����
// ��ü�Ǵ� ���� inorder successor (������ subtree���� ���� ������)
// ���� ���� ���´ٸ� ���������� �̵�!
int replaceK(rbnode *dc) {
	rbnode *replaced = dc->rchild;
	while (replaced->lchild)
		replaced = replaced->lchild;
	dc->key = replaced->key;
	return replaced->key;
}

// ���������� ������ �̷���� ������� �Ǵ�
bool possibleD(rbnode* node) {
	if (!node) return false;
	// lchild�� NULL Ȥ�� red leaf node && rchild�� NULL Ȥ�� red leaf node�϶� true
	if ((!(node->lchild) || (node->lchild && (node->lchild->red == true) && !(node->lchild->lchild) && !(node->lchild->rchild)))
		&& (!(node->rchild) || (node->rchild && (node->rchild->red == true) && !(node->rchild->lchild) && !(node->rchild->rchild))))
		return true;
	else return false;
}

// stocks�� 2 �̻��̸� stocks�� ���� -> ���� �Ϸ�
bool delstocks(rbnode* root, int k) { 
	root = root->lchild;
	while (root) {
		if (k == root->key) {
			if (root->stocks > 1) {
				root->stocks--;
				return true;
			}
			else return false;
		}
		else if (k < root->key) root = root->lchild;
		else root = root->rchild;
	}
	return false;
}

// ���� ��� ���� �Լ�
bool delete(rbnode* root, int k) {
	int value = k;
	rbnode *dgp, *dp, *dc, *dsb;
	dgp = dp = root;
	dc = root->lchild;
	dsb = NULL;
	if (delstocks(root, k) == true)
		return true;
	else {
		while (possibleD(dc) == false) {
			if (dc->red == false) { // dc�� black�̸� red�� ������ֱ�
				if (redtoP(root, dgp, dp, dsb) == true) {
					// redtoP ����Ǿ����� ��ġ ����
					dgp = dsb;
					if (dc->key > dp->key || dc->key == dp->key)
						dsb = dp->lchild;
					else dsb = dp->rchild;
				}
			}
			if (dc != root->lchild && isonlyblack(dc) == true) {
				// isonlyblack�̸� dc -> red node ������ֱ� -> �ؿ��� ���� �����ϵ���(���� ���� ����)
				if (redrotate(root, dgp, dp, dc, dsb) == false)
					reddown(dp);
			}
			// ������ �� ã�Ҵµ� ������ �� �ִ� �� �ƴ϶�� ��ü�� ã��
			if (value == dc->key) value = replaceK(dc);
			dgp = dp; dp = dc; // ��ġ ����
			if (value > dc->key || value == dc->key) {
				// dc, dsb ��ġ ����
				dsb = dc->lchild;
				dc = dc->rchild;
			}
			else {
				dsb = dc->rchild;
				dc = dc->lchild;
			}
		}
		// ���� ������ ��ġ �߰��ߴµ� black node�̸�
		if (dc->red == false) {
			// ���� �θ��� red�� �ٲ� �� ������ �ٲ���
			if (redtoP(root, dgp, dp, dsb) == true) {
				dgp = dsb;
				if (dc->key > dp->key || dc->key == dp->key)
					dsb = dp->lchild;
				else dsb = dp->rchild;
			}
		}
		// ���� ������ ��ġ �߰��߰� ��Ʈ��� �ƴѵ��� only black�̸�
		if (dc != root->lchild && isonlyblack(dc)) {
			if (redrotate(root, dgp, dp, dc, dsb) == false)
				reddown(dp);
		}
		if (dellast(root, value, dp, &dc) == true) return true;
		else return false;
	}
}