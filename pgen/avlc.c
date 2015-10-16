/******************************************************************************
 * AVL Balance Tree In C
 * Aaron Logue 2010 http://www.cryogenius.com/
 * This version stores key/pointer pairs.
 *****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "avlc.h"
#ifdef WIN32
#include "winstring.h"
#endif

/******************************************************************************
 * avlc_init()
 *****************************************************************************/
avlc_t * avlc_init(int flags) {
 avlc_t * avlc;
   avlc = (avlc_t *)malloc(sizeof(avlc_t));
   if (avlc) {
      avlc->root = NULL;
      avlc->count = 0;
      avlc->flags = flags;
   }
   return avlc;
}

/******************************************************************************
 * avlc_get_node()
 *****************************************************************************/
static void * avlc_get_node(avlc_node_t *node, char *key) {
 void * value = NULL;
 avlc_node_t * child;
 int relation;
   relation = strcasecmp(key, node->key);
   if (relation < 0) {
      child = node->node[0];
      if (child) {
         value = avlc_get_node(child, key);
      }
   } else {
      if (relation > 0) {
         child = node->node[1];
         if (child) {
            value = avlc_get_node(child, key);
         }
      } else {
         value = node->value;
      }
   }
   return value;
}

/******************************************************************************
 * avlc_get()
 * Descend the tree and return the first match encountered.
 *****************************************************************************/
void * avlc_get(avlc_t *tree, char *key) {
   if (tree && tree->root) {
      return avlc_get_node(tree->root, key);
   }
   return NULL;
}

/******************************************************************************
 * avlc_search_len()
 *****************************************************************************/
void * avlc_search_len(avlc_node_t *node, char *key, int len) {
 void * value = NULL;
 avlc_node_t * child;
 int relation;
   relation = strncasecmp(key, node->key, len);
   if (relation < 0) {
      child = node->node[0];
      if (child) {
         value = avlc_search_len(child, key, len);
      }
   } else
   if (relation > 0) {
      child = node->node[1];
      if (child) {
         value = avlc_search_len(child, key, len);
      }
   } else {
      value = node->value;
   }
   return value;
}

/******************************************************************************
 * avlc_get_len()
 * This version is handy when the caller doesn't have a null-terminated string
 * to work with. For example, a script parser looking for command tokens.
 *****************************************************************************/
void * avlc_get_len(avlc_t *tree, char *key, int len) {
   if (tree && tree->root && len > 0) {
      return avlc_search_len(tree->root, key, len);
   }
   return NULL;
}

/******************************************************************************
 * avlc_search_value()
 *****************************************************************************/
void * avlc_search_value(avlc_t * tree) {
 void * value = NULL;
 char * key;
 int relation;

   if (tree->node) {
      key = tree->key;
      relation = strncasecmp(key, tree->node->key, strlen(key));
      if (relation < 0) {
       avlc_node_t * node = tree->node;
         tree->node = node->node[0];
         value = avlc_search_value(tree);
         if (tree->flags & SUBTREE_FOUND) {
            // strcpy((char *)tree->value, node->key);
            tree->value = node->key;
            tree->flags &= ~SUBTREE_FOUND;
         }
      } else
      if (relation > 0) {
         tree->node = tree->node->node[1];
         value = avlc_search_value(tree);
      } else {
         if (tree->value == NULL) {
            value = tree->node->value;
         } else {
          avlc_node_t * node = tree->node;
         
            /* Try to find a match to the left of the match we've got */
            tree->node = node->node[0];
            value = avlc_search_value(tree);
            if (value) {
               if (tree->flags & SUBTREE_FOUND) {
                  // strcpy((char *)tree->value, node->key);
                  tree->value = node->key;
                  tree->flags &= ~SUBTREE_FOUND;
               }
            } else {
             int len;
             avlc_node_t *chase;

               len = strlen(key);
               chase = node->node[0];
               while (chase) {
                  if (!strncasecmp(key, chase->key, len)) {
                     break;
                  }
                  chase = chase->node[1];
               }
               if (chase == NULL) {
                  /* The last match we found was indeed the first in list */
                  /* Find next key by descending leftmost branch of right */
                  value = node->value;
                  node = node->node[1];
                  if (node) {
                     while (node->node[0]) {
                        node = node->node[0];
                     }
                     // strcpy((char *)tree->value, node->key);
                     tree->value = node->key;
                  } else {
                     /* Indicate to the last parent to descend to the left   */
                     /* child that their key is the next one after this one. */
                     tree->flags |= SUBTREE_FOUND;
                  }
               } else {
                  /* We found a new leftmost partial match. */
                  value = chase->value;
                  /* Descend to the next key, if one, and use it for next */
                  chase = chase->node[1];
                  if (chase) {
                     while (chase->node[0]) {
                        chase = chase->node[0];
                     }
                     // strcpy((char *)tree->value, chase->key);
                     tree->value = chase->key;
                  } else {
                     // strcpy((char *)tree->value, node->key);
                     tree->value = node->key;
                  }
               }

            }
         }
      }
   }
   return value;
}

/******************************************************************************
 * avlc_search()
 * Search for the first partially matching avlc value, and copy the next key
 * back to allow the caller to decide whether to continue searching or not.
 * This allows for implementation of tabbed completion.
 *****************************************************************************/
void * avlc_search(avlc_t *tree, char *key, char *next, int maxlen) {
 void *result = NULL;
   if (tree && tree->root) {
      tree->node  = tree->root;
      tree->key   = key;
      tree->value = next;
      result = avlc_search_value(tree);
      if (next) {
         if (tree->flags & SUBTREE_FOUND) {
            /* There is no next; found key is last one in tree. */
            *next = 0;
         } else {
            if (tree->value != next) {
               /* We found something to copy back */
               strncpy(next, (char *)tree->value, maxlen);
            }
         }
      }
   }
   return result;
}

/******************************************************************************
 * avlc_insert()
 *****************************************************************************/
avlc_node_t * avlc_insert(avlc_t *tree, avlc_node_t *node) {
 avlc_node_t *temp1, *temp2;
 int lidx, ridx, lunbal, runbal;
 int relation;

   if (node) {
      relation = strcasecmp(tree->key, node->key);
      if (relation == 0 && tree->flags & AVLC_ALLOW_DUPES) {
         relation = 1; /* Insert dupe key after the one already there */
      }
      if (relation) {
         lidx = 0;
         lunbal = -1;
         if (relation > 0) {
            /* Take advantage of L & R cases being mirror images */
            lidx = 1;
            lunbal = 1;
         }
         /* Set ridx and runbal to whatever lidx and lunbal aren't. */
         ridx = ~lidx & 1;
         runbal = lunbal * -1;

         /* Insert into child subtree and maybe change subtree root */
         node->node[lidx] = avlc_insert(tree, node->node[lidx]);
#ifdef TRACK_SUBTREE_SIZE
         if (tree->flags & SUBTREE_INCDEC_PARENT) {
            node->count++;
         }
#endif
         if (tree->flags & SUBTREE_CHANGED) {
            if (node->balance == 0) {
               /* Left branch grew and unbalanced node to left by 1. */
               /* Pass changed flag up to parent caller.             */
               node->balance = lunbal;
            } else {
               if (node->balance == lunbal) {
                  /* L branch grew and we were already unbalanced to left. */
                  /* We must balance and find a new root for this subtree. */
                  temp1 = node->node[lidx];   /* point temp1 at left child */
                  if (temp1->balance == lunbal) {
#ifdef TRACK_SUBTREE_SIZE
                     /* Node loses child and child's left subtree, if one. */
                     node->count--;
                     if (temp1->node[lidx]) {
                        node->count -= temp1->node[lidx]->count;
                     }
                     /* Child gains node and node's right subtree, if one */
                     temp1->count++;
                     if (node->node[ridx]) {
                        temp1->count += node->node[ridx]->count;
                     }
#endif
                     /* Single rotate case */
                     node->node[lidx]  = temp1->node[ridx];
                     temp1->node[ridx] = node;
                     node->balance = 0;
                     node = temp1;        /* L child = new subtree root */
                  } else {
                     /* Double rotate case */
                     temp2 = temp1->node[ridx];   /* L child's R child */
#ifdef TRACK_SUBTREE_SIZE
                     node->count -= temp1->count;
                     if (temp2->node[ridx]) {
                        node->count += temp2->node[ridx]->count;
                        temp1->count -= temp2->node[ridx]->count;
                     }
                     temp1->count--;
                     temp2->count = 1 + node->count + temp1->count;
#endif
                     temp1->node[ridx] = temp2->node[lidx];
                     temp2->node[lidx] = temp1;
                     node->node[lidx] = temp2->node[ridx];
                     temp2->node[ridx] = node;
                     if (temp2->balance == lunbal) {
                        node->balance = runbal;
                     } else {
                        node->balance = 0;
                     }
                     if (temp2->balance == runbal) {
                        temp1->balance = lunbal;
                     } else {
                        temp1->balance = 0;
                     }
                     node = temp2; /* L child's R child = new subtree root */
                  }
               }
               node->balance = 0;
               tree->flags &= ~SUBTREE_CHANGED;
            }
         }
      } else {
         /* Key already exists and dupes are not allowed. */
         tree->flags |= INSERT_FAILED;
      }
   } else {
      /* Create a new node and return it for insertion */
      node = (avlc_node_t *)malloc(sizeof(avlc_node_t));
      if (node) {
         node->key = (char *)malloc(strlen(tree->key)+1);
         if (node->key) {
            strcpy(node->key, tree->key);
         }
         node->value = tree->value;
         node->node[0] = NULL;
         node->node[1] = NULL;
         node->balance = 0;
#ifdef TRACK_SUBTREE_SIZE
         node->count   = 1;
         tree->flags |= SUBTREE_INCDEC_PARENT;
#endif
         tree->flags |= SUBTREE_CHANGED;
         tree->count++;
      }
   }
   return node;
}

/******************************************************************************
 * avlc_add()
 * Returns 1 if add successful, or 0 if attempt was made to add a
 * duplicate key and duplicate keys are not allowed.
 *****************************************************************************/
int avlc_add(avlc_t *tree, char *key, void *value) {
   if (tree) {
      tree->key   = key;
      tree->value = value;
      tree->flags = tree->flags & AVLC_ALLOW_DUPES;
      tree->root = avlc_insert(tree, tree->root);
      return !(tree->flags & INSERT_FAILED);
   }
   return 0; /* Failure due to tree not initialized */
}

/******************************************************************************
 * avlc_delete_balance()
 *****************************************************************************/
avlc_node_t * avlc_delete_balance(avlc_t *tree, avlc_node_t *node, int lidx) {
 avlc_node_t *temp1, *temp2;
 int ridx, lunbal, runbal;
   
   /* Take advantage of L & R cases being mirror images */
   lunbal = -1;
   if (lidx == 1) {
      lunbal = 1;
   }
   ridx = ~lidx & 1;
   runbal = lunbal * -1;

   if (node->balance == 0) {
      node->balance = runbal;
      tree->flags &= ~SUBTREE_CHANGED;
   } else {
      if (node->balance == lunbal) {
         node->balance = 0;
      } else {
         temp1 = node->node[ridx];
         if (temp1->balance != lunbal) {
            /* Single rotation */
#ifdef TRACK_SUBTREE_SIZE
            node->count -= temp1->count;
            if (temp1->node[lidx]) {
               node->count += temp1->node[lidx]->count;
            }
            temp1->count = 1 + node->count;
            if (temp1->node[ridx]) {
               temp1->count += temp1->node[ridx]->count;
            }
#endif            
            node->node[ridx] = temp1->node[lidx];
            temp1->node[lidx] = node;
            if (temp1->balance) {
               node->balance = 0;
               temp1->balance = 0;
            } else {
               node->balance = runbal;
               temp1->balance = lunbal;
               tree->flags &= ~SUBTREE_CHANGED;
            }
            node = temp1;
         } else {
            /* Double rotation */
            temp2 = temp1->node[lidx];
#ifdef TRACK_SUBTREE_SIZE
            node->count -= temp1->count;
            if (temp2->node[lidx]) {
               node->count += temp2->node[lidx]->count;
               temp1->count -= temp2->node[lidx]->count;
            }
            temp1->count--;
            temp2->count = 1 + node->count + temp1->count;
#endif
            temp1->node[lidx] = temp2->node[ridx];
            temp2->node[ridx] = temp1;
            node->node[ridx] = temp2->node[lidx];
            temp2->node[lidx] = node;
            if (temp2->balance == runbal) {
               node->balance = lunbal;
            } else {
               node->balance = 0;
            }
            if (temp2->balance == lunbal) {
               temp1->balance = runbal;
            } else {
               temp1->balance = 0;
            }
            node = temp2;
            node->balance = 0;
         }
      }
   }
   return node;
}

/******************************************************************************
 * avlc_delete_node()
 * This function is called when we have identified a node to delete,
 * and the node has elements in both its left and right subtrees.
 * We descend to the rightmost leaf of the node's left subtree and
 * link it in place of the deleted node, rebalancing on the way back.
 *****************************************************************************/
avlc_node_t * avlc_delete_node(avlc_t *tree, avlc_node_t *node) {
 char *temp;
 void *temp2;
   if (node->node[1]) {
      node->node[1] = avlc_delete_node(tree, node->node[1]);
#ifdef TRACK_SUBTREE_SIZE
      if (tree->flags & SUBTREE_INCDEC_PARENT) {
         node->count--;
      }
#endif
      if (tree->flags & SUBTREE_CHANGED) {
         node = avlc_delete_balance(tree, node, 1);
      }
   } else {

      temp = tree->node->key;
      tree->node->key = node->key;
      node->key = temp;
      temp2 = tree->node->value;
      tree->node->value = node->value;
      node->value = temp2;
      tree->node = node; /* free this leaf */

      node = node->node[0];

      tree->flags |= SUBTREE_CHANGED;
#ifdef TRACK_SUBTREE_SIZE
       tree->flags |= SUBTREE_INCDEC_PARENT;
#endif
   }
   return node;
}

/******************************************************************************
 * avlc_delete()
 *****************************************************************************/
avlc_node_t * avlc_delete(avlc_t *tree, avlc_node_t *node) {
 int relation;
   if (node) {
      relation = strcasecmp(tree->key, node->key);
      if (relation) {
       int idx;
         if (relation < 0) {
            idx = 0;
         } else {
            idx = 1;
         }
         node->node[idx] = avlc_delete(tree, node->node[idx]);
#ifdef TRACK_SUBTREE_SIZE
         if (tree->flags & SUBTREE_INCDEC_PARENT) {
            node->count--;
         }
#endif
         if (tree->flags & SUBTREE_CHANGED) {
            node = avlc_delete_balance(tree, node, idx);
         }
      } else {
         /* Delete this node. Hope for a non-merging case. */
         tree->node = node; /* this one gets new key/value */
         tree->value = node->value; /* Pass the value being deleted back */
         if (node->node[0] == NULL) {
            node = node->node[1];
            tree->flags |= SUBTREE_CHANGED;
#ifdef TRACK_SUBTREE_SIZE
            tree->flags |= SUBTREE_INCDEC_PARENT;
#endif
         } else {
            if (node->node[1] == NULL) {
               node = node->node[0];
               tree->flags |= SUBTREE_CHANGED;
#ifdef TRACK_SUBTREE_SIZE
               tree->flags |= SUBTREE_INCDEC_PARENT;
#endif
            } else {
               /* Node being deleted has both left and right subtrees. */
               node->node[0] = avlc_delete_node(tree, node->node[0]);
#ifdef TRACK_SUBTREE_SIZE
               if (tree->flags & SUBTREE_INCDEC_PARENT) {
                  node->count--;
               }
#endif
               if (tree->flags & SUBTREE_CHANGED) {
                  node = avlc_delete_balance(tree, node, 0);
               }
            }
         }

         if (tree->node->key) {
            free(tree->node->key);
         }
         free(tree->node);
         tree->node = NULL;
         tree->count--;
         tree->flags |= SUBTREE_FOUND;
      }
   } /* else, key not found - there is nothing to delete here */
   return node;
}

/******************************************************************************
 * avlc_del()
 *****************************************************************************/
void * avlc_del(avlc_t *tree, char *key) {
 void * result = NULL;
   if (tree && tree->root) {
      tree->key   = key;
      tree->value = NULL;
      tree->flags = tree->flags & AVLC_ALLOW_DUPES;
      tree->root  = avlc_delete(tree, tree->root);
      if (tree->flags & SUBTREE_FOUND) {
         result = tree->value;
      }
   }
   return result;
}

/******************************************************************************
 * avlc_walk_node()
 * Calls caller's function with elements to process tree in order.
 *****************************************************************************/
static int avlc_walk_node(avlc_node_t *node, avlc_func_t *func) {
 int rc = 0;
 avlc_node_t *child;
   child = node->node[0];
   if (child) {
      rc = avlc_walk_node(child, func);
   }
   if (!rc) {
      rc = func(node->key, node->value);
   }
   if (!rc) {
      child = node->node[1];
      if (child) {
         rc =  avlc_walk_node(child, func);
      }
   }
   return rc;
}

/******************************************************************************
 * avlc_walk()
 * This function performs a full walk of the tree.
 * Returns -1 if tree invalid, 0 if full walk, 1 if callback function stopped.
 *****************************************************************************/
int avlc_walk(avlc_t *tree, avlc_func_t *func) {
 int rc = -1;
   if (tree && tree->root) {
      rc = avlc_walk_node(tree->root, func);
   }
   return rc;
}

/******************************************************************************
 * avlc_walk_node_parm()
 * Calls caller's function with elements to process tree in order.
 * Passes a parameter to walker function. Stop if walker returns nonzero.
 *****************************************************************************/
static int avlc_walk_node_parm(avlc_node_t *node, avlc_func_parm_t *func, void *parm) {
 int rc = 0;
 avlc_node_t *child;
   child = node->node[0];
   if (child) {
      rc = avlc_walk_node_parm(child, func, parm);
   }
   if (!rc) {
      rc = func(node->key, node->value, parm);
   }
   if (!rc) {
      child = node->node[1];
      if (child) {
         rc = avlc_walk_node_parm(child, func, parm);
      }
   }
   return rc;
}

/******************************************************************************
 * avlc_walk_parm()
 * This performs a walk of the tree with a parameter to pass to the caller's
 * function.  The caller's function can stop the walk by returning nonzero.
 *****************************************************************************/
int avlc_walk_parm(avlc_t *tree, avlc_func_parm_t *func, void *parm) {
 int rc = -1;
   if (tree && tree->root) {
      rc = avlc_walk_node_parm(tree->root, func, parm);
   }
   return rc;
}

/******************************************************************************
 * avlc_free_node()
 *****************************************************************************/
void avlc_free_node(avlc_node_t *node) {
   if (node) {
      avlc_free_node(node->node[0]);
      avlc_free_node(node->node[1]);
      if (node->key) {
         free(node->key);
      }
      free(node);
   }
}

/******************************************************************************
 * avlc_free()
 * This does not free any data structures pointed at by the stored
 * void *s.  To do that, write a function to free your structure:
 *    void my_struct_free_func(char * key, void * value) {
 *     my_struct_t * my_struct;
 *       my_struct = (my_struct_t *)value;
 *       if (my_struct->stuff_ptr) {
 *          free(my_struct->stuff_ptr);
 *       }
 *       free(my_struct);
 *    }
 * Then walk the tree to free your data before you call this function:
 *    avlc_walk(tree_instance, &my_struct_free_func);
 *    avlc_free(tree_instance);
 *****************************************************************************/
int avlc_free(avlc_t *tree) {
   if (tree) {
      avlc_free_node(tree->root);
      free(tree);
   }
   return 1;
}

#ifdef TRACK_SUBTREE_SIZE
/******************************************************************************
 * avlc_get_node_by_index()
 *****************************************************************************/
static avlc_node_t * avlc_get_node_by_index(avlc_node_t *node, int index) {
 int leftsize;
   if (node) {
      leftsize = 0;
      if (node->node[0]) {
         leftsize = node->node[0]->count;
      }
      if (index <= leftsize) {
         node = avlc_get_node_by_index(node->node[0], index);
      } else {
         index -= leftsize;
         if (index > 1) {
            index--;
            node = avlc_get_node_by_index(node->node[1], index);
         }
      }
   }
   return node;
}

/******************************************************************************
 * avlc_get_key_by_index()
 *****************************************************************************/
char * avlc_get_key_by_index(avlc_t *tree, int index) {
 char *result = NULL;
 avlc_node_t *node;
   if (tree && index > 0 && index <= tree->count) {
      node = avlc_get_node_by_index(tree->root, index);
      if (node) {
         result = node->key;
      }
   }
   return result;
}

/******************************************************************************
 * avlc_get_value_by_index()
 *****************************************************************************/
void * avlc_get_value_by_index(avlc_t *tree, int index) {
 void *result = NULL;
 avlc_node_t *node;
   if (tree && index > 0 && index <= tree->count) {
      node = avlc_get_node_by_index(tree->root, index);
      if (node) {
         result = node->value;
      }
   }
   return result;
}

/******************************************************************************
 * avlc_find_index()
 * *index contains how many are to our left on each call.
 * When we hit the matching leaf, we return the negative of the
 * found index to indicate success.  The caller will negate again
 * to return a positive success to the user, and a negative or zero
 * to indicate failure.
 *****************************************************************************/
void * avlc_find_index(avlc_node_t *node, char *key, int *index) {
 void * value = NULL;
 int relation;
 avlc_node_t *child;

   relation = strcasecmp(key, node->key);
   if (relation < 0) {
      /* The number of nodes to the left of the subtree is unchanged */
      if (node->node[0]) {
         value = avlc_find_index(node->node[0], key, index);
      }
   } else {
      if (relation > 0) {
         /* Tell subtree how many are to its left by subtracting  */
         /* their count from ours and adding however many we were */
         /* told were to our left */
         if (node->node[1]) {
            *index += node->count - node->node[1]->count;
            value = avlc_find_index(node->node[1], key, index);
         }
      } else {

       avlc_node_t *match = NULL;
       avlc_node_t *chase = node->node[0];
       int row = *index; /* how many are to our left */
         /* Before we settle on current matching node, check to see if */
         /* there are any duplicate key matches to our left. */
         /* Ideally, we would only bother with this if dupes are allowed. */
         /* Start by matching left until we hit a node that does not match */
         while (chase && !strcasecmp(key, chase->key)) {
            match = chase;
            chase = chase->node[0];
         }
         /* If no match, follow left child's right branch looking for match */
         if (!match && chase) {
            /* Add the no-match node plus size of its left subtree */
            child = chase->node[0];
            if (child) {
               row += child->count + 1;
            }
            chase = chase->node[1];
            while (chase && strcasecmp(key, chase->key)) {
               /* Add the node we're skipping plus size of its left subtree */
               child = chase->node[0];
               if (child) {
                  row += child->count + 1;
               }
               chase = chase->node[1];
            }
            match = chase; /* We either matched or NULLed out */
         }
         if (match) {
            node = match;
            *index = row;
         }

         value = node->value;
         /* Add the size of the subtree to our left */
         child = node->node[0];
         if (child) {
            *index += child->count;
         }
         /* Add one for us and negate to indicate found */
         *index = 0 - (*index + 1);
      }
   }
   return value;
}

/******************************************************************************
 * avlc_get_value_and_index()
 * This function takes a tree instance, a key, and a pointer to an integer to
 * receive the index.  It searches for the key and returns the pointer value
 * and fills in the caller's index with a positive 1-based value if found.
 * If the key is not found, the returned index value is negative or zero.
 *****************************************************************************/
void * avlc_get_value_and_index(avlc_t *tree, char *key, int *index) {
 void * value = NULL;
   if (tree && tree->root && key && index) {
      *index = 0; /* There are no nodes to the left of the subtree */
      value = avlc_find_index(tree->root, key, index);
      *index = 0 - *index;
   }
   return value;
}

/******************************************************************************
 * avlc_delete_index()
 * Descend the tree in much the same way that avlc_delete() does, only using
 * the target index rather than key comparisons.  Use the same functions
 * avlc_delete_balance() and avlc_delete_node() that it does.
 *****************************************************************************/
static avlc_node_t * avlc_delete_index(avlc_t *tree, avlc_node_t *node, int index) {
 int leftsize;
   if (node) {
      leftsize = 0;
      if (node->node[0]) {
         leftsize = node->node[0]->count;
      }
      if (index <= leftsize) {
         node->node[0] = avlc_delete_index(tree, node->node[0], index);
         if (tree->flags & SUBTREE_INCDEC_PARENT) {
            node->count--;
         }
         if (tree->flags & SUBTREE_CHANGED) {
            node = avlc_delete_balance(tree, node, 0);
         }
      } else {
         index -= leftsize;
         if (index > 1) {
            index--;
            node->node[1] = avlc_delete_index(tree, node->node[1], index);
            if (tree->flags & SUBTREE_INCDEC_PARENT) {
               node->count--;
            }
            if (tree->flags & SUBTREE_CHANGED) {
               node = avlc_delete_balance(tree, node, 1);
            }
         } else {
            /* Delete this node and its key. */
            tree->node = node;
            tree->value = node->value; /* Pass the deleted value back */
            if (node->node[0] == NULL) {
               node = node->node[1];
               tree->flags |= SUBTREE_CHANGED | SUBTREE_INCDEC_PARENT;
            } else {
               if (node->node[1] == NULL) {
                  node = node->node[0];
                  tree->flags |= SUBTREE_CHANGED | SUBTREE_INCDEC_PARENT;;
               } else {
                  /* Node being deleted has two subtrees. */
                  node->node[0] = avlc_delete_node(tree, node->node[0]);
                  if (tree->flags & SUBTREE_INCDEC_PARENT) {
                     node->count--;
                  }
                  if (tree->flags & SUBTREE_CHANGED) {
                     node = avlc_delete_balance(tree, node, 0);
                  }
               }
            }
            if (tree->node->key) {
               free(tree->node->key);
            }
            free(tree->node);
            tree->node = NULL;
            tree->count--;
         }
      }
   } /* else, node counts are corrupted! */
   return node;
}

/******************************************************************************
 * avlc_del_index()
 * Delete the node with the ordinal value of index.
 * Returns the void * of the deleted node so that the caller can free it.
 * Returns NULL if the index was not found, which should only happen if it
 * was out of range or if the node counts are corrupted.
 *****************************************************************************/
void * avlc_del_index(avlc_t *tree, int index) {
 void *result = NULL;
   if (tree && tree->root && index > 0 && index <= tree->count) {
      tree->flags = tree->flags & AVLC_ALLOW_DUPES;
      tree->value = NULL;
      tree->root = avlc_delete_index(tree, tree->root, index);
      result = tree->value;
   }
   return result;
}

/******************************************************************************
 * avlc_walk_range_node()
 *****************************************************************************/
static int avlc_walk_range_node(avlc_node_t *node, avlc_func_parm_t *func,
                                void *parm, int first, int last) {
 int rc = 0;
 int leftsize = 0;
 avlc_node_t *child;
   child = node->node[0];
   if (child) {
      leftsize = child->count;
      if (first <= leftsize) {
         rc = avlc_walk_range_node(child, func, parm, first, last);
      }
   }
   if (!rc) {
      first -= leftsize + 1;
      last -= leftsize + 1;
      if (first <= 0 && last >= 0) {
         rc = func(node->key, node->value, parm);
      }
      if (!rc && node->node[1] && last > 0) {
         rc = avlc_walk_range_node(node->node[1], func, parm, first, last);
      }
   }
   return rc;
}

/******************************************************************************
 * avlc_walk_range()
 * This function calls the caller's function with elements to process
 * tree in order, stopping when element index last is reached.
 * first and last are 1-based, so valid ranges are 1 to tree->count
 *****************************************************************************/
int avlc_walk_range(avlc_t *tree, avlc_func_parm_t *func, void *parm, int first, int last) {
 int rc = -1;
   if (tree && tree->root && first > 0 && last <= tree->count && first <= last) {
      rc = avlc_walk_range_node(tree->root, func, parm, first, last);
   }
   return rc;
}
#endif
