/******************************************************************************
 * AVL Balance Tree In C
 * Aaron Logue 2010 http://www.cryogenius.com/
 * This version stores key/pointer pairs
 *
 * Defining TRACK_SUBTREE_SIZE causes a count of nodes below and including
 * the current node to be maintained in each node.  This allows elements to
 * be accessed by their position within the tree as well as by their key.
 *****************************************************************************/
#define TRACK_SUBTREE_SIZE

#define SUBTREE_CHANGED       1
#define SUBTREE_INCDEC_PARENT 2
#define SUBTREE_FOUND         4
#define INSERT_FAILED         8
#define AVLC_ALLOW_DUPES      16

typedef struct avlc_node_tt {
   char *        key;
   void *        value;
   struct avlc_node_tt * node[2];
   int           balance;
#ifdef TRACK_SUBTREE_SIZE
   int           count;  /* How many nodes are in this subtree */
#endif
} avlc_node_t;

typedef struct avlc_tt {
   avlc_node_t * root;
   avlc_node_t * node;   /* Passes a constant node ptr through recurs dels */
   char *        key;    /* Key and value are passed here to save pushes */
   void *        value;
   unsigned int  flags;  /* Reports subtree height changes to parent node */
   int           count;  /* Number of elements in tree */
} avlc_t;

/******************************************************************************
 * avlc_func_t and avlc_func_parm_t are the typedefs for callback functions
 * that you specify to the tree walking functions.  All of the callback
 * functions can halt the walk by returning 1, or can return 0 to keep walking.
 * The walking function avlc_walk() takes no parameter to pass to the callback
 * function, but avlc_walk_parm() and avlc_walk_range() do.
 *****************************************************************************/
typedef int avlc_func_t(char *key, void *value);
typedef int avlc_func_parm_t(char *key, void *value, void *parm);

/******************************************************************************
 * avlc_init()
 * Call this function to create and initialize a tree.
 * This function allocates an instance of a tree and returns a pointer to it.
 * You then pass that pointer into other avlc_ functions to tell them which
 * tree you want to operate on.  Thus, your program can use multiple trees
 * simultaneously.
 * Input: AVLC_ALLOW_DUPES causes avlc_add() to allow duplicate keys in the tree.
 *        Otherwise, it'll refuse to add duplicates and will return 0.
 *****************************************************************************/
avlc_t * avlc_init(int flags);

/******************************************************************************
 * avlc_add()
 * Returns 1 for successful insert, or 0 if duplicate key.
 * This function allocates memory to store a copy of the key
 * and also allocates memory for the node that is created.
 *****************************************************************************/
int      avlc_add(avlc_t *tree, char *key, void *value);

/******************************************************************************
 * avlc_get()
 * Given a key, retrieve the value.  Returns NULL if key not found.
 *****************************************************************************/
void *   avlc_get(avlc_t *tree, char *key);

/******************************************************************************
 * avlc_del()
 * Delete the key and node associated with it..
 * It is the caller's responsibility to delete the data structure pointed
 * at by value, if any. The node and the copy of the key are freed.
 * The value associated with the node is passed back if delete was successful,
 * or NULL otherwise.
 *****************************************************************************/
void *   avlc_del(avlc_t *tree, char *key);

/******************************************************************************
 * avlc_free()
 * Call this function to destroy a tree.
 * This does not free any data structures pointed at by the stored
 * void *s.  To do that, write a function to free your structure:
 *    int my_struct_free_func(char * key, void * value) {
 *     my_struct_t * my_struct;
 *       my_struct = (my_struct_t *)value;
 *       if (my_struct->stuff_ptr) {
 *          free(my_struct->stuff_ptr);
 *       }
 *       free(my_struct);
 *       return 0;
 *    }
 * Then walk the tree to free your data before you call this function:
 *    avlc_walk(tree_instance, &my_struct_free_func);
 *    avlc_free(tree_instance);
 *****************************************************************************/
int      avlc_free(avlc_t *tree);

/******************************************************************************
 * avlc_walk()
 *****************************************************************************/
int      avlc_walk(avlc_t *tree, avlc_func_t *func);

/******************************************************************************
 * avlc_walk_parm()
 *****************************************************************************/
int      avlc_walk_parm(avlc_t *tree, avlc_func_parm_t *func, void *parm);

/******************************************************************************
 *****************************************************************************/
void *   avlc_search(avlc_t *tree, char *key, char *next, int maxlen);

/******************************************************************************
 *****************************************************************************/
void *   avlc_get_len(avlc_t *tree, char * key, int len);

#ifdef TRACK_SUBTREE_SIZE
/******************************************************************************
 * avlc_get_value_and_index()
 * This function takes a tree instance, a key, and a pointer to an integer to
 * receive the index.  It searches for the key and returns the pointer value
 * and fills in the caller's index with a positive 1-based value if found.
 * If the key is not found, the returned index value is negative or zero.
 *****************************************************************************/
void *   avlc_get_value_and_index(avlc_t *tree, char * key, int * index);

/******************************************************************************
 *****************************************************************************/
char *   avlc_get_key_by_index(avlc_t *tree, int index);

/******************************************************************************
 *****************************************************************************/
void *   avlc_get_value_by_index(avlc_t *tree, int index);

/******************************************************************************
 *****************************************************************************/
void *   avlc_del_index(avlc_t *tree, int index);

/******************************************************************************
 *****************************************************************************/
int      avlc_walk_range(avlc_t *tree, avlc_func_parm_t *func, void *parm,
                         int first, int last);
#endif
