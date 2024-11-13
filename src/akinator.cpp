#include "akinator.h"
#include "tree_dump.h"

node_t* new_node(int key, node_t* left, node_t* right, char* data)
{
    node_t* node = (node_t*)calloc(1, sizeof(node_t));
    if (!node)
    {
        fprintf(stderr, "ERROR: Calloc returned NULL in new_node()");
        return NULL;
    }
    node -> key   = key;
    node -> left  = left;
    node -> right = right;
    strcpy(node -> data, data);

    return node;
}

int fix_parents(node_t* node)
{
    assert(node);
    if (node -> right) {(node -> right) -> parent = node;}
    if (node -> left)  {(node -> left) ->  parent = node;}

    return 0;
}

int find_node(node_t* node, FILE* html_stream, node_t* root)
{
    fprintf(stdout, "%s?\n", node -> data);

    char ans = 0;
    node_t** temp = NULL;
    get_next_node(&temp, node, &ans);

    if (*temp)
    {
        if ((*temp) -> left == NULL && ((*temp) -> right) == NULL)
        {
            printf("It is %s, right?\n", (*temp) -> data);
            get_ans(&ans);

            if (ans == 'N')
            {
                add_node(temp);
                printf("An object was added succesfully\n");
                tree_dump(root, html_stream, node);
            }
            else { printf("Object: %s\nAkinator end\n", (*temp)->data);}
            return 0;
        }
        else
        {
            find_node(*temp, html_stream, root);
            return 0;
        }
    }

    return -1;
}

int get_next_node(node_t*** temp, node_t* node, char* ans)
{
    assert(node);

    while (true)
    {
        fscanf(stdin, "%c", ans);
        clean_input_buffer();
        if      (*ans == 'Y') { *temp = &node -> right; break; }
        else if (*ans == 'N') { *temp = &node -> left;  break; }
        else {printf("Unknown symbol: %c(%d)\nOnly Y and N are accepted as answers\n", *ans, *ans);}
    }

    return 0;
}

int get_ans(char* ans)
{
    assert(ans);

    while (true)
    {
        fscanf(stdin, "%c", ans);
        clean_input_buffer();

        if (!(*ans == 'Y' || *ans == 'N'))
            printf("Unknown symbol: %c(%d)\nOnly Y and N are accepted as answers\n", *ans, *ans);
        else
            break;
    }

    return 0;
}

int add_node(node_t** temp)
{
    assert(temp);

    printf("What was it?\n");
    char new_label[default_str_size] = {};
    scanf(" %s[^\n]\n", new_label);

    printf("what is the difference between %s and %s?(Enter the question)\n"
            "(True for %s, false for %s)\n",
            new_label, (*temp) -> data, new_label, (*temp) -> data);

    char question[default_str_size] = {};
    scanf(" %s[^\n]\n", question);

    node_t* question_node = new_node(0, NULL, NULL, question);
    node_t* label_node    = new_node(0, NULL, NULL, new_label);

    question_node -> right = label_node;
    question_node -> left  = *temp;

    node_t* parent = (*temp) -> parent;

    *temp = question_node;

    fix_parents(parent);
    fix_parents(question_node);

    return 0;
}

int give_label_def(node_t* root)
{
    assert(root);

    printf("Enter name of object\n");
    char label[default_str_size] = {};
    scanf(" %s", label);
    node_t* node = node_label_search(root, label);
    if (!node)
    {
        printf("There is no such object in this database\n");
        return -1;
    }

    stack_t path_stack = {};
    stack_init(&path_stack, 16, sizeof(stack_elem_t));
    get_path(node, &path_stack);
    //STACK_DUMP(&path_stack, __func__);

    print_label_def(root, &path_stack, label);

    return 0;
}

int get_path(node_t* node, stack_t* path_stack)
{
    assert(node);
    assert(path_stack);

    while (node -> parent)
    {
        if ((node -> parent) -> right == node)     {stack_push(path_stack, 1);}
        else if ((node -> parent) -> left == node) {stack_push(path_stack, 0);}

        node = node -> parent;
    }

    return 0;
}

node_t* node_label_search(node_t* node, char* label)
{
    assert(node);
    assert(label);

    if (strcmp(node -> data, label) == 0) {return node;}
    else
    {
        node_t* ans = NULL;
        if (node -> right && (ans = node_label_search(node -> right, label))) {return ans;}
        if (node -> left  && (ans = node_label_search(node -> left, label)))  {return ans;}
    }

    return NULL;
}

int print_label_def(node_t* node, stack_t* path_stack, char label[default_str_size])
{
    assert(node);
    assert(path_stack);

    size_t path_length = path_stack -> size;
    int direction = 0;

    printf("Definition of %s:\n", label);
    for (size_t i = 0; i < path_length; i++)
    {
        stack_pop(path_stack, &direction);
        if (direction == 1)
        {
            printf("    %s\n", node -> data);
            node = node -> right;
        }
        else
        {
            printf("Not %s\n", node -> data);
            node = node -> left;
        }
    }

    return 0;
}

int compare(node_t* root)
{
    assert(root);

    printf("Enter first label: ");
    char label1[default_str_size] = {};
    scanf(" %s", label1);

    printf("Enter second label: ");
    char label2[default_str_size] = {};
    scanf(" %s", label2);

    node_t* node1 = node_label_search(root, label1);
    node_t* node2 = node_label_search(root, label2);

    if (!node1) {fprintf(stderr, "Error: no such item(%s)\n", label1); return -1;}
    if (!node2) {fprintf(stderr, "Error: no such item(%s)\n", label2); return -1;}

    stack_t path_stack1 = {};
    stack_init(&path_stack1, 16, sizeof(stack_elem_t));
    get_path(node1, &path_stack1);

    stack_t path_stack2 = {};
    stack_init(&path_stack2, 16, sizeof(stack_elem_t));
    get_path(node2, &path_stack2);

    printf("\n%s and %s similarities:\n", label1, label2);

    node_t* node = root;

    print_similar_features(&node, &path_stack1, &path_stack2);

    node_t* node_copy = node;

    printf("%s different features:\n", label1);
    print_features(node, &path_stack1);
    printf("%s different features:\n", label2);
    print_features(node_copy, &path_stack2);

    return 0;
}

int print_similar_features(node_t** node, stack_t* path_stack1, stack_t* path_stack2)
{
    assert(node);
    assert(path_stack1);
    assert(path_stack2);

    int direction1 = 0;
    int direction2 = 0;
    while (path_stack1 -> size > 0 && path_stack2 -> size > 0)
    {
        stack_pop(path_stack1, &direction1);
        stack_pop(path_stack2, &direction2);
        if (direction1 != direction2)
        {
            stack_push(path_stack1, direction1);
            stack_push(path_stack2, direction2);
            break;
        }
        if (direction1 == 1)
        {
            printf("    %s\n", (*node) -> data);
            (*node) = (*node) -> right;
        }
        else
        {
            printf("Not %s\n", (*node) -> data);
            (*node) = (*node) -> left;
        }
    }

    return 0;
}

int print_features(node_t* node, stack_t* path_stack)
{
    assert(node);
    assert(path_stack);

    int direction = 0;
    while (path_stack -> size > 0)
    {
        stack_pop(path_stack, &direction);
        if (direction == 1)
        {
            printf("    %s\n", node -> data);
            node = node -> right;
        }
        else
        {
            printf("Not %s\n", node -> data);
            node = node -> left;
        }
    }

    return 0;
}

int read_tree(node_t** node, FILE* stream, FILE* html_stream)
{
    assert(stream);
    char ch = 0;
    fscanf(stream, " %c", &ch);
    //printf("ch: %c\nnode : %p\n", ch, *node);
    switch (ch)
    {
        case '{':
        {
            char label[default_str_size] = {};
            fscanf(stream, " \"%[^\"]\"", label);
            //printf("%s\n", label);
            *node = new_node(0, NULL, NULL, label);
            read_tree(node, stream, html_stream);
            break;
        }
        case 'y':
        {
            char temp = 0;
            while(temp != '{')
                fscanf(stream, "%c", &temp);

            char label[default_str_size] = {};
            fscanf(stream, " \"%[^\"]\"", label);
            //printf("%s\n", label);
            (*node) -> right = new_node(0, NULL, NULL, label);
            ((*node) -> right) -> parent = *node;

            read_tree(&((*node) -> right), stream, html_stream);
        }
        case 'n':
        {
            char temp = 0;
            while(temp != '{')
                fscanf(stream, "%c", &temp);

            char label[default_str_size] = {};
            fscanf(stream, " \"%[^\"]\"", label);
            //printf("%s\n", label);
            (*node) -> left = new_node(0, NULL, NULL, label);
            ((*node) -> left) -> parent = *node;

            read_tree(&((*node) -> left), stream, html_stream);
            break;
        }
        case '}': {return 0;}
        default:{fprintf(stderr, "Unknown option: %c\n", ch); break;}
    }

    //print_tree(*node, stdout);
    return 0;
}

#define TAB_FPRINTF(k, stream, ...)      \
    for (int i = 0; i < (k); i++)    \
        fprintf(stream, "\t");          \
    fprintf(stream , __VA_ARGS__);      \


int write_tree(node_t* node, FILE* stream, int depth)
{
    assert(stream);

    TAB_FPRINTF(depth, stream, "{\n");
    TAB_FPRINTF(depth + 1, stream,"\"%s\"\n", node -> data);

    if (node -> right)
    {
        TAB_FPRINTF(depth + 1, stream, "yes:\n");
        write_tree(node -> right, stream, depth + 1);
    }
    if (node -> left)
    {
        TAB_FPRINTF(depth + 1, stream, "no:\n");
        write_tree(node -> left, stream, depth + 1);
    }
    TAB_FPRINTF(depth, stream, "}\n");

    return 0;
}

#undef TAB_FPRINTF

int print_tree(node_t* node, FILE* stream)
{
    assert(stream);

    if (!node) { return 1; }

    fprintf(stream, "{");
    fprintf(stream, "%s", node -> data);
    if (node -> left)  { print_tree(node -> left, stream);  }
    if (node -> right) { print_tree(node -> right, stream); }
    fprintf(stream, "}");

    return 0;
}

int tree_dtor(node_t* node)
{
    if (!node) { return 1; }
    if (node -> left)  { tree_dtor(node -> left);  }
    if (node -> right) { tree_dtor(node -> right); }
    free(node);

    return 0;
}
