/* Do not remove the headers from this file! see /USAGE for more info. */

#define CLUSTER_NAVIGATION
#undef CLUSTER_COMBAT

/* Do not touch anything below this point*/

#define CLUSTERS "/std/behaviour/clusters/"
#define NODE_CLASS "/std/behaviour/node_class"

#ifndef __BEHAVIOR_H__
#define __BEHAVIOR_H__

#define EVAL_NONE -1
#define EVAL_SUCCESS 1
#define EVAL_RUNNING 2
#define EVAL_FAILURE 0

// Composite nodes
#define NODE_ROOT 1
#define NODE_SEQUENCE 2
#define NODE_SELECTOR 3

// Leaf
#define NODE_LEAF 5

// Decorator nodes
#define NODE_SUCCEEDER 10
#define NODE_INVERTER 11
#define NODE_REPEAT_UNTIL_FAIL 12

#endif /* __BEHAVIOUR_H__ */