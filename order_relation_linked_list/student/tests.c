#include <stdlib.h>
#include <stdio.h>
#include "student_code.h"
#include "CTester/CTester.h"

int cmp1(char a, char b){
    return a - b;
}

int cmp2(char a, char b){
    return b - a;
}

int cmp3(char a, char b){
    int res = (a%2 > 0) - (b%2 > 0);
    if(!res) return a-b;
    return res;
}

void free_list(node_t** h){
    node_t* runner = *h;
    node_t* rm;
    while (runner) {
        rm = runner;
        runner = runner->next;
        free(rm);
    }
}

/*
* @header: beginning of the list
* @begin_char: first char expected in the list
* @order_relation: 1 if natural order, -1 if reverse order
* @size_list: number of elements expected in the list
* @offset_inserted: position expected of the inserted element
*
* @return:  0 if correct Insertion
*           -1 if node are in bad order
*           -2 if the number of elements
*           -3 if inserted element is not malloced
*/
int test_list(node_t** header, char begin_char, int order_relation, int size_list, int offset_inserted){
    int count = 1;
    char cmp = begin_char;
    node_t* runner = *header;
    while(runner){
        if(count == offset_inserted) if(!malloced(runner)) return -3;
        if(runner->val != cmp) return -1;
        count++;
        cmp = cmp + order_relation;
        runner = runner->next;
    }
    if(count != size_list + 1) return -2;
    return 0;
}

void test_compare_equals() {
    set_test_metadata("order_relation", _("Comparison of equals char"), 1);

    int ret = 1;

    monitored.malloc = true;
    monitored.free = true;
    SANDBOX_BEGIN;
    ret = compare('a','a');
    SANDBOX_END;

    CU_ASSERT_EQUAL(ret,0);

    ret = 1;

    SANDBOX_BEGIN;
    ret = compare('z','z');
    SANDBOX_END;

    CU_ASSERT_EQUAL(ret,0);

    ret = 1;

    SANDBOX_BEGIN;
    ret = compare('\0','\0');
    SANDBOX_END;

    CU_ASSERT_EQUAL(ret,0);

    CU_ASSERT_EQUAL(stats.malloc.called,0);

    CU_ASSERT_EQUAL(stats.free.called,0);
    if (stats.free.called || stats.malloc.called) {
        push_info_msg(_("Why do you use malloc or free in compare"));
    }
}

void test_compare_greater() {
    set_test_metadata("order_relation", _("Comparison with a > b"), 1);

    int ret = 0;

    monitored.malloc = true;
    monitored.free = true;
    SANDBOX_BEGIN;
    ret = compare('a','Z');
    SANDBOX_END;

    CU_ASSERT_TRUE(ret > 0);

    ret = 0;

    SANDBOX_BEGIN;
    ret = compare('z','y');
    SANDBOX_END;

    CU_ASSERT_TRUE(ret > 0);

    ret = 0;

    SANDBOX_BEGIN;
    ret = compare('\n','\0');
    SANDBOX_END;

    CU_ASSERT_TRUE(ret > 0);

    CU_ASSERT_EQUAL(stats.malloc.called,0);

    CU_ASSERT_EQUAL(stats.free.called,0);
    if (stats.free.called || stats.malloc.called) {
        push_info_msg(_("Why do you use malloc or free in compare"));
    }
}

void test_compare_smaller() {
    set_test_metadata("order_relation", _("Comparison with a < b"), 1);

    int ret = 0;

    monitored.malloc = true;
    monitored.free = true;
    SANDBOX_BEGIN;
    ret = compare('Z','a');
    SANDBOX_END;

    CU_ASSERT_TRUE(ret < 0);

    ret = 0;

    SANDBOX_BEGIN;
    ret = compare('y','z');
    SANDBOX_END;

    CU_ASSERT_TRUE(ret < 0);

    ret = 0;

    SANDBOX_BEGIN;
    ret = compare('\0','\n');
    SANDBOX_END;

    CU_ASSERT_TRUE(ret < 0);

    CU_ASSERT_EQUAL(stats.malloc.called,0);

    CU_ASSERT_EQUAL(stats.free.called,0);
    if (stats.free.called || stats.malloc.called) {
        push_info_msg(_("Why do you use malloc or free in compare"));
    }
}

void test_empty_list(){
    set_test_metadata("insert", _("Insertion in an empty list"), 1);

    int ret = 1;
    node_t* l = NULL;
    node_t** h = &l;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'c',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_TRUE(malloced(*h));
    if(!malloced(*h)) CU_FAIL("You must allocate dynamic memory on the heap rather then static variables on the stack");

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    if(*h){
        CU_ASSERT_EQUAL((*h)->val,'c');
        CU_ASSERT_PTR_NULL((*h)->next);
    }
    else{
        CU_FAIL(_("You must change the head to your created node"));
    }
    free_list(h);
}

void test_empty_list_fail(){
    set_test_metadata("insert", _("Insertion in an empty list with malloc fail"), 1);

    int ret = 1;
    node_t* l = NULL;
    node_t** h = &l;

    monitored.malloc = 1;
    failures.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'c',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, 0);
    if(delta) CU_FAIL(_("You shouldn't allocate memory if malloc fails"));

    CU_ASSERT_EQUAL(ret,-1);
    if(!ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);

    if(*h) CU_FAIL(_("The head sould be NULL"));
}

void test_insert_first_cresc(){
    set_test_metadata("insert", _("Insertion in first place, natural order"), 1);

    int ret = 1;
    node_t* l = (node_t*) malloc(sizeof(node_t));
    if(!l) { CU_FAIL(_("no mem")); return;}
    l->val = 'b';
    l->next = NULL;
    node_t** h = &l;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'a',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_TRUE(malloced(*h));
    if(!malloced(*h)) CU_FAIL("You must allocate dynamic memory on the heap rather then static variables on the stack");

    ret = test_list(h,'a',1,2,1);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test_insert_first_fails(){
    set_test_metadata("insert", _("Insertion in first place with malloc failure"), 1);

    int ret = 1;
    node_t* l = (node_t*) malloc(sizeof(node_t));
    if(!l) { CU_FAIL(_("no mem")); return;}
    l->val = 'c';
    l->next = NULL;
    node_t** h = &l;

    monitored.malloc = 1;
    failures.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'a',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, 0);
    if(delta) CU_FAIL(_("You shouldn't allocate memory if malloc fails"));

    CU_ASSERT_EQUAL(ret,-1);
    if(!ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    if(*h){
        CU_ASSERT_EQUAL((*h)->val,'c');
        CU_ASSERT_PTR_NULL((*h)->next);
        if((*h)->next){
            CU_FAIL(_("You change the list if malloc fails"));
        }
    }
    else CU_FAIL(_("You souldn't change the head"));

    free_list(h);
}

void test1_insert_middle_cresc(){
    set_test_metadata("insert", _("Insertion in middle place, simple case, natural order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    l2->val = 'c';
    l2->next = NULL;
    l1->val = 'a';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'b',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'a',1,3,2);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test2_insert_middle_cresc(){
    set_test_metadata("insert", _("Insertion in middle place, complex case, natural order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    node_t* l3 = (node_t*) malloc(sizeof(node_t));
    if(!l3) { CU_FAIL(_("no mem")); free(l1); free(l2); return;}
    node_t* l4 = (node_t*) malloc(sizeof(node_t));
    if(!l4) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); return;}
    node_t* l5 = (node_t*) malloc(sizeof(node_t));
    if(!l5) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); free(l4); return;}
    l5->val = 'f';
    l5->next = NULL;
    l4->val = 'd';
    l4->next = l5;
    l3->val = 'c';
    l3->next = l4;
    l2->val = 'b';
    l2->next = l3;
    l1->val = 'a';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'e',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'a',1,6,5);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test1_insert_middle_fails(){
    set_test_metadata("insert", _("Insertion in middle place, with malloc failure"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    l2->val = 'c';
    l2->next = NULL;
    l1->val = 'a';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;
    failures.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'b',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, 0);
    if(delta) CU_FAIL(_("You shouldn't allocate memory when malloc fails"));

    CU_ASSERT_EQUAL(ret,-1);
    if(!ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    if(*h){
        CU_ASSERT_EQUAL((*h)->val,'a');
        CU_ASSERT_PTR_NOT_NULL((*h)->next);
        if((*h)->next){
            CU_ASSERT_EQUAL((*h)->next->val,'c');
            CU_ASSERT_PTR_NULL((*h)->next->next);
        }
        else CU_FAIL(_("You must not change the list"));
    }
    else CU_FAIL(_("You must not change the list"));

    free_list(h);
}

void test1_insert_last_cresc(){
    set_test_metadata("insert", _("Insertion in last place, simple case, natural order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    l2->val = 'b';
    l2->next = NULL;
    l1->val = 'a';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'c',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'a',1,3,3);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test2_insert_last_cresc(){
    set_test_metadata("insert", _("Insertion in last place, complex case, natural order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    node_t* l3 = (node_t*) malloc(sizeof(node_t));
    if(!l3) { CU_FAIL(_("no mem")); free(l1); free(l2); return;}
    node_t* l4 = (node_t*) malloc(sizeof(node_t));
    if(!l4) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); return;}
    node_t* l5 = (node_t*) malloc(sizeof(node_t));
    if(!l5) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); free(l4); return;}
    l5->val = 'e';
    l5->next = NULL;
    l4->val = 'd';
    l4->next = l5;
    l3->val = 'c';
    l3->next = l4;
    l2->val = 'b';
    l2->next = l3;
    l1->val = 'a';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'f',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'a',1,6,6);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }
    free_list(h);
}

void test1_insert_last_fails(){
    set_test_metadata("insert", _("Insertion in last place, with malloc failure"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    l2->val = 'b';
    l2->next = NULL;
    l1->val = 'a';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;
    failures.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'c',&cmp1);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, 0);
    if(delta) CU_FAIL(_("You shouldn't allocate memory when malloc fails"));

    CU_ASSERT_EQUAL(ret,-1);
    if(!ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    if(*h){
        CU_ASSERT_EQUAL((*h)->val,'a');
        CU_ASSERT_PTR_NOT_NULL((*h)->next);
        if((*h)->next){
            CU_ASSERT_EQUAL((*h)->next->val,'b');
            CU_ASSERT_PTR_NULL((*h)->next->next);
        }
        else CU_FAIL(_("You must not change the list"));
    }
    else CU_FAIL(_("You must not change the list"));

    free_list(h);
}

void test_insert_first_decresc(){
    set_test_metadata("insert", _("Insertion in first place, reverse order"), 1);

    int ret = 1;
    node_t* l = (node_t*) malloc(sizeof(node_t));
    if(!l) { CU_FAIL(_("no mem")); return;}
    l->val = 'y';
    l->next = NULL;
    node_t** h = &l;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'z',&cmp2);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_TRUE(malloced(*h));
    if(!malloced(*h)) CU_FAIL("You must allocate dynamic memory on the heap rather then static variables on the stack");

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'z',-1,2,1);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test1_insert_middle_decresc(){
    set_test_metadata("insert", _("Insertion in middle place, simple case, reverse order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    l2->val = 'x';
    l2->next = NULL;
    l1->val = 'z';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'y',&cmp2);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'z',-1,3,2);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test2_insert_middle_decresc(){
    set_test_metadata("insert", _("Insertion in middle place, complex case, reverse order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    node_t* l3 = (node_t*) malloc(sizeof(node_t));
    if(!l3) { CU_FAIL(_("no mem")); free(l1); free(l2); return;}
    node_t* l4 = (node_t*) malloc(sizeof(node_t));
    if(!l4) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); return;}
    node_t* l5 = (node_t*) malloc(sizeof(node_t));
    if(!l5) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); free(l4); return;}
    l5->val = 'u';
    l5->next = NULL;
    l4->val = 'w';
    l4->next = l5;
    l3->val = 'x';
    l3->next = l4;
    l2->val = 'y';
    l2->next = l3;
    l1->val = 'z';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'v',&cmp2);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'z',-1,6,5);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test1_insert_last_decresc(){
    set_test_metadata("insert", _("Insertion in last place, simple case, reverse order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    l2->val = 'y';
    l2->next = NULL;
    l1->val = 'z';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'x',&cmp2);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'z',-1,3,3);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}

void test2_insert_last_decresc(){
    set_test_metadata("insert", _("Insertion in last place, complex case, reverse order"), 1);

    int ret = 1;
    node_t* l1 = (node_t*) malloc(sizeof(node_t));
    if(!l1) { CU_FAIL(_("no mem")); return;}
    node_t* l2 = (node_t*) malloc(sizeof(node_t));
    if(!l2) { CU_FAIL(_("no mem")); free(l1); return;}
    node_t* l3 = (node_t*) malloc(sizeof(node_t));
    if(!l3) { CU_FAIL(_("no mem")); free(l1); free(l2); return;}
    node_t* l4 = (node_t*) malloc(sizeof(node_t));
    if(!l4) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); return;}
    node_t* l5 = (node_t*) malloc(sizeof(node_t));
    if(!l5) { CU_FAIL(_("no mem")); free(l1); free(l2); free(l3); free(l4); return;}
    l5->val = 'v';
    l5->next = NULL;
    l4->val = 'w';
    l4->next = l5;
    l3->val = 'x';
    l3->next = l4;
    l2->val = 'y';
    l2->next = l3;
    l1->val = 'z';
    l1->next = l2;
    node_t** h = &l1;

    monitored.malloc = 1;

    size_t start = stats.memory.used;
    SANDBOX_BEGIN;
    ret = insert(h,'u',&cmp2);
    SANDBOX_END;
    size_t delta = stats.memory.used - start;

    CU_ASSERT_EQUAL(delta, sizeof(node_t));
    if(delta != sizeof(node_t)) CU_FAIL(_("You don't allocate the good amount of memory"));

    CU_ASSERT_EQUAL(ret,0);
    if(ret) CU_FAIL(_("You don't return the good value"));

    CU_ASSERT_PTR_NOT_NULL(*h);

    CU_ASSERT_EQUAL(stats.malloc.called,1);
    if(stats.malloc.called != 1) CU_FAIL(_("You only have to call malloc once to insert a node"));

    ret = test_list(h,'z',-1,6,6);

    switch (ret) {
        case -1:
            CU_FAIL(_("The elements are not in the correct order"));
            break;
        case -2:
            CU_FAIL(_("There is not the correct number of elements in the list"));
            break;
        case -3:
            CU_FAIL(_("The inserted element is not malloced"));
            break;
        case 0:
            break;
        default:
            break;
    }

    free_list(h);
}


int main(int argc,char** argv)
{
    BAN_FUNCS(free);
    RUN(test_compare_equals, test_compare_greater, test_compare_smaller,test_empty_list, test_empty_list_fail, test_insert_first_cresc, test_insert_first_fails, test1_insert_middle_cresc, test2_insert_middle_cresc, test1_insert_middle_fails, test1_insert_last_cresc,test2_insert_last_cresc, test1_insert_last_fails, test_insert_first_decresc, test1_insert_middle_decresc, test2_insert_middle_decresc, test1_insert_last_decresc, test2_insert_last_decresc);
}
