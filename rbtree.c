#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// red black tree 정의
typedef struct rbnode_ {
	int key;
	struct rbnode_* lchild; struct rbnode_* rchild;
	bool red; // true면 red node, 아니면 black node
	unsigned long long stocks;// 중복 허용 위해 key 몇개 있는지
}rbnode;

//노드 생성자
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
bool search(rbnode* root, int key); // 찾으면 true 못찾으면 false
void insert(rbnode* root, int key);
bool delete(rbnode* root, int key); // 지우면 true 해당 데이터 없었으면 false

// rbt 내부 함수
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

// 메모리 할당 해제 함수
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

	rbtroot = new_node(); // red black tree의 root -> 비어있고 ㅣchild부터 씀
	rbtroot->rchild = rbtroot; rbtroot->key = 0; rbtroot->stocks = -1; // dummy node

	while (1) {
		res = fscanf(ifp, "%c %d", &func, &k);
		if (res == EOF)
			break; // 다읽었으면 리턴
		if (func == 'i') { // 삽입
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

//bst와 탐색 방법 같음
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
	// key -> 크기 가지고 roatate 방향 정함 (gchild쪽 값)
	rbnode *child, *gchild;
	// pv의 child 찾아주기
	if ((k > pv->key || k == pv->key) && pv != root)
		child = pv->rchild;
	else
		child = pv->lchild;
	// 왼쪽 회전
	if (k > child->key || k == child->key) {
		gchild = child->rchild; // gchild 찾아주기
		child->rchild = gchild->lchild;
		gchild->lchild = child;
	}
	else {// 오른쪽 회전
		gchild = child->lchild; // gchild 찾아주기
		child->lchild = gchild->rchild;
		gchild->rchild = child;
	}

	// pv에 어디에 붙일지 정한다.
	if ((k > pv->key || k == pv->key) && pv != root)
		pv->rchild = gchild;
	else
		pv->lchild = gchild;
	// 회전 후 pv의 자식노드 반환
	return gchild;
}

// stocks가 2 이상이면 stocks만 감소 -> 삭제 완료
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
	rbnode *c, *p, *gp, *ggp; // 자식, 부모, 조부모, 증조부모 순서 유지
	p = gp = ggp = root;
	c = root->lchild;
	if (addstocks(root, k) == true)
		return;
	else {
		while (c) {
			if (c->lchild && c->rchild && c->lchild->red == true && c->rchild->red == true) {
				// 밑에 두 자식이 red node -> 상향색전환
				c->red = true;
				c->lchild->red = c->rchild->red = false;
				if (p->red == true) {
					// red node 연달아 나왔음 -> 회전해줘야함
					gp->red = true; // 모든 경우에서 gp는 red로 바뀜
					if ((k > gp->key) != (k > p->key))
						// 위에서 꺾이는 경우에는 double rotate 해줌
						p = rotate(root, k, gp);
					//위에서 안꺾이면 그냥 rotate 한번
					c = rotate(root, k, ggp);
					c->red = false; // 모든 경우에서 c는 black으로 바뀜
				}
				// root는 항상 black
				root->lchild->red = false;
			}
			// 선두 순서 유지
			ggp = gp; gp = p; p = c;
			if (k > c->key) c = c->rchild;
			else c = c->lchild;
		}
		// 알맞은 위치의 leaf node 찾음-> 노드 새로 만들기
		c = new_node();
		c->key = k;
		c->red = true; // 새로 삽입되는 노드는 red
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
		// root는 항상 검정
		root->lchild->red = false;
		return;
	}
}

void removenode(rbnode** node) {
	// 자료가 1개 있을때 삭제 후 메모리 해제
	rbnode* temp = *node;
	*node = NULL;
	free(temp);
	return;
}

bool dellast(rbnode* root, int k, rbnode* dp, rbnode** dc) { // 지우고자 하는 키, 지우고자하는 부모노드, 지우고자하는 노드 자신
	if (k == (*dc)->key && !((*dc)->lchild) && !((*dc)->rchild)) {
		// dc, 지우고자하는 값이 leaf 노드인 경우(red) -> black node는 지울수 없음 (color demotion -> 하향 색전환)
		removenode(dc);
		// 부모노드의 링크 연결 끊기
		if ((k > dp->key || k == dp->key) && dp != root) // 내부노드의 삭제 원칙(부모가 key랑 같아질수도 있음)
			// 나는 지금 오른쪽 subtree에서 가장 작은 값(inorder successor)을 위로 올렸기 때문에 키값과 부모 키값이 같을 경우 오른쪽으로 이동
			dp->rchild = NULL;
		else dp->lchild = NULL;
		return true;
	}
	else if (k == (*dc)->key) {
		// dc, 지우고자하는 값이 black node인 경우(leaf node의 부모)
		rbnode* sub = NULL; // 원래 위치(dc)에 대신 들어갈 것
		if ((*dc)->lchild) {
			// lchild가 있으면 lchild를 부모 밑에 붙일것
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
	// 블랙노드가 dc이고 지우고자 하는 값은 dc의 자식노드(red node)일때
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
	else return false; // 삭제 실패
}

// dc-> 지우는 과정에 있는 현재노드
// 탐색 경로의 부모노드를 항상 red node로 유지 해줌
bool redtoP(rbnode* root, rbnode* dgp, rbnode* dp, rbnode* dsb) {
	if (dsb == NULL || dsb->red == false) // 형제노드가 없거나 형제노드가 black node이면
		return false;
	// dp, dsb rotate -> dsb가 dp의 부모, dc의 조부모 됨
	rotate(root, dsb->key, dgp);
	dsb->red = false;
	// dc의 부모인 dp는 red node가 됨
	dp->red = true;
	return true;
}

// (부모는 원래 붙어있던 곳에서 분리시키고) 부모, 형제와 결합 -> 하향 색전환
// 조건: 현재 노드와 sibling이 black이고, 두 자식가지며 둘다 black => 안그러면 합쳐질수 없음
void reddown(rbnode* dp) {
	// 부모는 black
	dp->red = false;
	// 자식들은 red
	dp->lchild->red = dp->rchild->red = true;
	return;
}

// 자신, 자식 노드가 모두 black인지 확인
bool isonlyblack(rbnode* node) {
	if (!node) return false;
	if (node->red == true) return false;
	if ((!(node->lchild) && !(node->rchild)) ||
		(node->lchild&&node->rchild && (node->lchild->red == false) && (node->rchild->red == false)))
		return true;
	else return false;
}

// red node 가진 형제 => rotate해서 현재노드를 red로
// 조건: sibling: black, 하나이상의 red 자식 가짐
bool redrotate(rbnode* root, rbnode* dgp, rbnode* dp, rbnode* dc, rbnode* dsb) {
	rbnode *dsbrc; // dsb의 red 자식노드
	if (isonlyblack(dsb)==true) return false; // red node 없으면 조건 불충족
	if (dc->key > dsb->key) { // dsb가 왼쪽, dc가 오른쪽 자식노드
		if (dsb->lchild && dsb->lchild->red == true)
			// 왼쪽 자식으로 red node가 있을경우
			dsbrc = dsb->lchild;
		else dsbrc = dsb->rchild;
	}
	else { // dsb가 오른쪽, dc가 왼쪽
		if (dsb->rchild && dsb->rchild->red == true)
			// red node인 오른쪽 자식이 있을 경우
			dsbrc = dsb->rchild;
		else dsbrc = dsb->lchild;
	}
	if ((dp->key > dsb->key) == (dsb->key > dsbrc->key)) {
		// 중간에 안꺾여있으면 회전 한번
		rotate(root, dsbrc->key, dgp);
		dsb->red = true;
		dsbrc->red = false;
	}
	else { // 꺾였으면 회전 두번
		rotate(root, dsbrc->key, dp);
		rotate(root, dsbrc->key, dgp);
		dsb->red = false;
		dsbrc->red = true;
	}
	dc->red = true;
	dp->red = false;
	// root는 항상 black
	if (root->lchild->red == true)
		root->lchild->red = false;
	return true;
}

// 중간에 있는 값 삭제를 위한 구현
// 중간에 있는 값을 맨 밑 노드의 값으로 대체 -> 그 leaf노드를 삭제
// 대체되는 값은 inorder successor (오른쪽 subtree에서 가장 작은값)
// 같은 값이 나온다면 오른쪽으로 이동!
int replaceK(rbnode *dc) {
	rbnode *replaced = dc->rchild;
	while (replaced->lchild)
		replaced = replaced->lchild;
	dc->key = replaced->key;
	return replaced->key;
}

// 실질적으로 삭제가 이루어질 노드인지 판단
bool possibleD(rbnode* node) {
	if (!node) return false;
	// lchild가 NULL 혹은 red leaf node && rchild가 NULL 혹은 red leaf node일때 true
	if ((!(node->lchild) || (node->lchild && (node->lchild->red == true) && !(node->lchild->lchild) && !(node->lchild->rchild)))
		&& (!(node->rchild) || (node->rchild && (node->rchild->red == true) && !(node->rchild->lchild) && !(node->rchild->rchild))))
		return true;
	else return false;
}

// stocks가 2 이상이면 stocks만 감소 -> 삭제 완료
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

// 최종 노드 삭제 함수
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
			if (dc->red == false) { // dc가 black이면 red로 만들어주기
				if (redtoP(root, dgp, dp, dsb) == true) {
					// redtoP 수행되었으면 위치 갱신
					dgp = dsb;
					if (dc->key > dp->key || dc->key == dp->key)
						dsb = dp->lchild;
					else dsb = dp->rchild;
				}
			}
			if (dc != root->lchild && isonlyblack(dc) == true) {
				// isonlyblack이면 dc -> red node 만들어주기 -> 밑에서 삭제 가능하도록(삭제 공간 마련)
				if (redrotate(root, dgp, dp, dc, dsb) == false)
					reddown(dp);
			}
			// 삭제할 값 찾았는데 삭제할 수 있는 곳 아니라면 대체값 찾기
			if (value == dc->key) value = replaceK(dc);
			dgp = dp; dp = dc; // 위치 갱신
			if (value > dc->key || value == dc->key) {
				// dc, dsb 위치 갱신
				dsb = dc->lchild;
				dc = dc->rchild;
			}
			else {
				dsb = dc->rchild;
				dc = dc->lchild;
			}
		}
		// 삭제 가능한 위치 발견했는데 black node이면
		if (dc->red == false) {
			// 위에 부모노드 red로 바꿀 수 있으면 바꿔줌
			if (redtoP(root, dgp, dp, dsb) == true) {
				dgp = dsb;
				if (dc->key > dp->key || dc->key == dp->key)
					dsb = dp->lchild;
				else dsb = dp->rchild;
			}
		}
		// 삭제 가능한 위치 발견했고 루트노드 아닌데도 only black이면
		if (dc != root->lchild && isonlyblack(dc)) {
			if (redrotate(root, dgp, dp, dc, dsb) == false)
				reddown(dp);
		}
		if (dellast(root, value, dp, &dc) == true) return true;
		else return false;
	}
}