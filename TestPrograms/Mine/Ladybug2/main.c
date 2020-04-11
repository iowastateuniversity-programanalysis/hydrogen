#include <stdio.h>

struct ladybug {
    int num_wings;
    int num_dots;
};

// A ladybug has 3 dots per wing
void get_num_dots(struct ladybug *this) {
    this->num_dots = this->num_wings *= 3;
}

int main() {
    struct ladybug bug;
    scanf("%d", &bug.num_wings);

    if (bug.num_wings <= 0) {
        printf("A ladybug gotta have wings, ya dig?\n");
    }

    get_num_dots(&bug);

    printf("A %d-winged ladybug has %d dots\n", bug.num_wings, bug.num_dots);
}
