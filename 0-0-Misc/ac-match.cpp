
// Aho-Corasick automation
//   match string with patterns by AC automation.
//   by BOT Man & ZhangHan, 2018

/*
  Example:
    PATTERN
      he
      she
      his
      hers
    STRING
      ushers
      she
    OUTPUT
      1  2  she
      1  3  he
      1  3  hers
      2  2  he
*/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: remove C++ header by implementing a simple queue
#include <queue>

// format: line column string
#define OUTPUT_FORMAT "%5d %5d  %s\n"

// NodeValue should align with input characters
typedef char NodeValue;

/*
  Link list illustrated:
    parent                        parent
      ^    ^       ^                ^        ^          ^
      |     \       \         =>    |         \          \
      ~      ~       ~              ~          \          \
    child0  child1  child2...    children -> sibling -> sibling ...
*/
typedef struct _ACNode {
  // for goto
  NodeValue value;
  struct _ACNode *children; // head of link list
  struct _ACNode *sibling;  // next of link list

  // for fail
  struct _ACNode *fail_link;

  // for output
  struct _ACNode *parent;
  uint16_t depth; // used to allocation stack memory

  // is current node end of pattern string (this node is output node)
  uint8_t is_node_output;

  // is there output node along fail link (this node may not be output node)
  uint8_t is_fail_link_output;
} ACNode;

// used to check memory leaks of node
static size_t g_node_count;

/*
  Create new node with value.
*/
ACNode *NewNode(NodeValue value) {
  ACNode *node = (ACNode *)malloc(sizeof(ACNode));
  assert(node);
  if (!node)
    return NULL;

  ++g_node_count;

  memset(node, 0, sizeof(ACNode));
  node->value = value;
  return node;
}

/*
  Delete nodes on the tree.
*/
void DeleteNode(ACNode *node) {
  if (!node)
    return;

  // delete children link list
  ACNode *child = node->children;
  while (child) {
    ACNode *next = child->sibling;
    DeleteNode(child);
    child = next;
  }

  // delete this node
  free((void*)node);
  --g_node_count;
}

/*
  Add child to parent.
*/
void AddChild(ACNode *parent, ACNode *child) {
  if (!parent || !child)
    return;

  // check if parent has children already
  ACNode *children_list = parent->children;
  if (children_list) {
    // find last child of children list
    ACNode *last_child = children_list;
    while (last_child->sibling)
      last_child = last_child->sibling;

    // set child to last child's sibling
    last_child->sibling = child;
  } else {
    // set child to the first of children list
    parent->children = child;
  }

  // set parent of child
  child->parent = parent;

  // set depth of child
  child->depth = (uint16_t)(parent->depth + 1);
}

/*
  Find child of parent.
*/
ACNode *FindChild(ACNode *parent, NodeValue expected_value) {
  if (!parent)
    return NULL;

  ACNode *child = parent->children;
  while (child) {
    // found if child has expected value
    if (child->value == expected_value)
      break;

    // move to sibling
    child = child->sibling;
  }
  return child;
}

/*
  Construct output string of output node.
*/
const char *CreateOutputString(ACNode *node) {
  if (!node || !node->is_node_output)
    return NULL;

  /*
      [ 0 0 0 0 ]
              ^
      [ 0 0 x 0 ]
            ^ ^
      [ 0 y x 0 ]
          ^   ^
      [ z y x 0 ]
        ^     ^
  */

  size_t stack_size = (size_t)(node->depth + 1);
  size_t memory_size = sizeof(NodeValue) * stack_size;

  NodeValue *output_stack = (NodeValue *)malloc(memory_size);
  assert(output_stack);
  if (!output_stack)
    return NULL;

  // point to the last element (bottom of stack)
  NodeValue *top_of_stack = &output_stack[stack_size - 1];

  // set string ending 0 to output_stack
  *top_of_stack = 0;

  // trace back until current node is root (parent == NULL)
  ACNode *current_node = node;
  while (current_node && current_node->parent) {
    // move top ptr
    --top_of_stack;

    assert(top_of_stack >= output_stack);
    if (top_of_stack < output_stack)
      break;

    // push value of node to stack
    *top_of_stack = current_node->value;

    // move to parent
    current_node = current_node->parent;
  }

  // ensure ouput stack is fulfilled
  assert(top_of_stack == output_stack);

  return output_stack;
}

/*
  Destruct output string.
*/
void DeleteOutputString(const char *str) {
  if (!str)
    return;

  free((void *)str);
}

/*
  AC step1: build tire tree by pattern strings.
*/
ACNode *BuildTireTree(const char *pattern, ACNode *root) {
  if (!pattern || !root)
    return NULL;

  // iterate over str
  ACNode *leaf = root;
  for (const char *ch = pattern; ch && *ch != '\n' && *ch != '\r'; ++ch) {
    NodeValue value = *ch;

    // find existing child first
    ACNode *found_child = FindChild(leaf, value);
    if (found_child) {
      // set leaf to found child
      leaf = found_child;
    } else {
      // add new node to leaf
      ACNode *new_node = NewNode(value);
      AddChild(leaf, new_node);

      // set leaf to new node
      leaf = new_node;
    }
  }

  // mark final leaf as output node
  leaf->is_node_output = (leaf != root);

  return leaf;
}

/*
  AC step2: construct fail link and output link of tree.
*/
void InitFailLink(ACNode *root) {
  if (!root)
    return;

  std::queue<ACNode *> queue;

  // set layer1 (depth == 1) fail link to root
  ACNode *child = root->children;
  while (child) {
    // set fail link to root
    child->fail_link = root;

    // push child to queue
    queue.push(child);

    // move to next child
    child = child->sibling;
  }

  // set fail link by BFS
  while (!queue.empty()) {
    ACNode *node = queue.front();
    queue.pop();

    ACNode *child = node->children;
    while (child) {
      // find a state along the fail links with valid goto
      ACNode *state = node->fail_link;
      while (state && !FindChild(state, child->value))
        state = state->fail_link;

      if (state) {
        // set fail link to the goto-state if found
        child->fail_link = FindChild(state, child->value);
        assert(child->fail_link);

        // mark as fail-link-output if fail link is
        child->is_fail_link_output = child->fail_link->is_node_output ||
                                     child->fail_link->is_fail_link_output;
      } else {
        // set fail link to root if no goto-state found
        child->fail_link = root;
      }

      // push child to queue
      queue.push(child);

      // move to next child
      child = child->sibling;
    }
  }
}

/*
  Try to print all outputs on matching a node.
*/
void CheckOutputOnMatch(ACNode *node, uint32_t line_num, uint32_t column_num,
                        FILE *output_file) {
  if (!node || !output_file)
    return;

  // print current node
  if (node->is_node_output) {
    const char *output_string = CreateOutputString(node);
    uint32_t offset = column_num - (uint32_t)strlen(output_string) + 1;
    fprintf(output_file, OUTPUT_FORMAT, line_num, offset, output_string);
    DeleteOutputString(output_string);
  }

  // print outputs along fail link
  if (node->is_fail_link_output) {
    ACNode *next = node->fail_link;
    while (next) {
      // check next node
      CheckOutputOnMatch(next, line_num, column_num, output_file);

      // move to next node along fail link
      next = next->fail_link;
    }
  }
}

/*
  AC step3: use tire tree with fail link to match a line.
*/
void ACMatch(const char *line, uint32_t line_num, ACNode *root,
             FILE *output_file) {
  if (!line || !root)
    return;

  uint32_t column_num = 1; // start with 1
  ACNode *current = root;
  for (const char *ch = line; ch && *ch != '\n' && *ch != '\r';) {
    NodeValue value = *ch;

    // find goto by current value
    ACNode *found_child = FindChild(current, value);
    if (found_child) {
      // check output on current state
      CheckOutputOnMatch(found_child, line_num, column_num, output_file);

      // move to next goto-state
      current = found_child;

      // forward str
      ++ch;
      ++column_num;
    } else {
      // root->fail_link == NULL (could not move to any more)
      if (current == root) {
        // forward str
        ++ch;
        ++column_num;
      } else {
        // move along fail link (without forwarding str)
        current = current->fail_link;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "usage: ./acmatch STRING PATTERN OUTPUT\n");
    return 1;
  }

  FILE *string_file = fopen(argv[1], "r");
  FILE *pattern_file = fopen(argv[2], "r");
  FILE *output_file = fopen(argv[3], "w");

  if (!string_file || !pattern_file || !output_file) {
    if (string_file)
      fclose(string_file);
    else
      fprintf(stderr, "failed to open string file: %s\n", argv[1]);

    if (pattern_file)
      fclose(pattern_file);
    else
      fprintf(stderr, "failed to open pattern file: %s\n", argv[2]);

    if (output_file)
      fclose(output_file);
    else
      fprintf(stderr, "failed to open output file: %s\n", argv[3]);

    return 1;
  }

  // create root node of tire tree
  ACNode *root = NewNode(0);

  // build tire tree by pattern line
  {
    char *line = NULL;
    size_t line_len = 0;
    while (-1 != getline(&line, &line_len, pattern_file))
      BuildTireTree(line, root);
  }

  // init tire tree's fail function
  InitFailLink(root);

  // use AC to match string line
  {
    char *line = NULL;
    size_t line_len = 0;
    uint32_t line_num = 0; // start with 1
    while (-1 != getline(&line, &line_len, string_file)) {
      ++line_num;
      ACMatch(line, line_num, root, output_file);
    }
  }

  // delete the tire tree
  DeleteNode(root);

  fclose(string_file);
  fclose(pattern_file);
  fclose(output_file);

  // ensure no memory leaks of node
  assert(g_node_count == 0);
  return 0;
}
