#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <../libuthread/queue.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Callback function that increments integer items by a certain value (or delete
 * item if item is value 42) */
static int inc_item(queue_t q, void *data, void *arg)
{
    int *a = (int*)data;
    int inc = (int)(long)arg;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += inc;

    return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg)
{
    int *a = (int*)data;
    int match = (int)(long)arg;
    (void)q; //unused

    if (*a == match)
        return 1;

    return 0;
}

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;
	int ret;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	ret = queue_enqueue(q, &data);
	TEST_ASSERT(ret == 0);
	ret = queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ret == 0);
	TEST_ASSERT(ptr == &data);
	ret = queue_destroy(q);
	TEST_ASSERT(ret == 0);
}

/* Enqueue */
void test_queue_enqueue(void)
{
	int data = 3;
	int ret;
	queue_t q;

	fprintf(stderr, "*** TEST queue_enqueue null***\n");
	ret = queue_enqueue(NULL, &data);
	TEST_ASSERT(ret == -1);
	q = queue_create();
	ret = queue_enqueue(q, NULL);
	TEST_ASSERT(ret == -1);
}

/* Dequeue */
void test_queue_dequeue(void)
{
	int *ptr;
	int ret;
	queue_t q;

	fprintf(stderr, "*** TEST queue_dequeue null***\n");
	ret = queue_dequeue(NULL, (void**)&ptr);
	TEST_ASSERT(ret == -1);
	q = queue_create();
	ret = queue_dequeue(q, NULL);
	TEST_ASSERT(ret == -1);
	fprintf(stderr, "*** TEST queue_dequeue empty queue ***\n");
	ret = queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ret == -1);
}

/* Destory */
void test_destory(void)
{
    queue_t q;
    int data = 1;
	int ret;
	int* ptr;
	
	q = queue_create();

	fprintf(stderr, "*** TEST queue_destory null ***\n");
	ret = queue_destroy(NULL);
	TEST_ASSERT(ret == -1);

	fprintf(stderr, "*** TEST queue_destory non-empty ***\n");
    queue_enqueue(q, &data);
	ret = queue_destroy(q);
    TEST_ASSERT(ret == -1);

	fprintf(stderr, "*** TEST queue_destory empty ***\n");
	queue_dequeue(q, (void**)&ptr);
	ret = queue_destroy(q);
	TEST_ASSERT(ret == 0);
}


/* Delete */
void test_queue_delete(void)
{
	int data[] = {3,4,5,6};
	int *ptr;
	size_t i;
	queue_t q;
	int ret;

	q = queue_create();

	fprintf(stderr, "*** TEST queue_delete null***\n");
	ret = queue_delete(NULL, &data[1]);
	TEST_ASSERT(ret == -1);
	ret = queue_delete(q, NULL);
	TEST_ASSERT(ret == -1);
	ret = queue_delete(q, &data[3]);
	TEST_ASSERT(ret == -1);

	fprintf(stderr, "*** TEST queue_delete ***\n");
	
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);
	queue_delete(q, &data[1]);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data[0]);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data[2]);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data[3]);

	/* Free queue */
	int r = queue_destroy(q);
	TEST_ASSERT(r == 0);
}

/* Iterate */
void test_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;
	int ret;

	q = queue_create();
	fprintf(stderr, "*** TEST queue_iterator null***\n");
	ret = queue_iterate(NULL, inc_item, (void*)1, NULL);
	TEST_ASSERT(ret == -1);
	ret = queue_iterate(q, NULL, (void*)1, NULL);
	TEST_ASSERT(ret == -1);


	fprintf(stderr, "*** TEST queue_iterator ***\n");

    /* Initialize the queue and enqueue items */
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Add value '1' to every item of the queue, delete item '42' */
    ret = queue_iterate(q, inc_item, (void*)1, NULL);
	TEST_ASSERT(ret == 0);
    TEST_ASSERT(data[0] == 2);

	fprintf(stderr, "*** TEST queue_length ***\n");
	ret = queue_length(NULL);
	TEST_ASSERT(ret == -1);
	ret = queue_length(q);
    TEST_ASSERT(ret == 9);

    /* Find and get the item which is equal to value '5' */
    ptr = NULL;     // result pointer *must* be reset first
    ret =queue_iterate(q, find_item, (void*)5, (void**)&ptr);
	TEST_ASSERT(ret == 0);


	/* Dequeuing everything in the queue and destory after */
	size_t cur_queue_size = (size_t)queue_length(q);
    for (i =0; i < cur_queue_size; i++){
        queue_dequeue(q, (void**)&ptr);
	}	
	queue_destroy(q);
	
}



int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_enqueue();
	test_queue_dequeue();
	test_destory();
	test_queue_delete();
	test_iterator();
	
	return 0;
}
