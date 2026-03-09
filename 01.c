#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE_LEN 100
#define AUTHOR_LEN 50
#define FILENAME "library.dat"

typedef struct Book {
    int id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    int copies; 
    struct Book *next;
} Book;

Book *head = NULL;
 
void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

char *strcasestr_compat(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    size_t nlen = strlen(needle);
    for (; *haystack; haystack++) {
        size_t i;
        for (i = 0; i < nlen; i++) {
            char a = haystack[i], b = needle[i];
            if (!b) break;
            if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
            if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
            if (a != b) break;
        }
        if (i == nlen) return (char *)haystack;
    }
    return NULL;
}

void load_books() {
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp) return;
    Book tmp;
    while (fread(&tmp, sizeof(Book), 1, fp) == 1) {
        Book *b = (Book*)malloc(sizeof(Book));
        if (!b) { perror("malloc"); fclose(fp); return; }
        *b = tmp;
        b->next = head;
        head = b;
    }
    fclose(fp);
}

void save_books() {
    FILE *fp = fopen(FILENAME, "wb");
    if (!fp) { perror("fopen"); return; }
    Book *cur = head;
    while (cur) {
        Book out = *cur;
        out.next = NULL;
        if (fwrite(&out, sizeof(Book), 1, fp) != 1) {
            perror("fwrite");
            break;
        }
        cur = cur->next;
    }
    fclose(fp);
}

int next_id() {
    int max = 0;
    Book *cur = head;
    while (cur) {
        if (cur->id > max) max = cur->id;
        cur = cur->next;
    }
    return max + 1;
}

void add_book() {
    char title[TITLE_LEN], author[AUTHOR_LEN];
    int copies;

    printf("Enter book title: ");
    fgets(title, TITLE_LEN, stdin); trim_newline(title);
    if (strlen(title) == 0) { printf("Title cannot be empty.\n"); return; }

    printf("Enter author name: ");
    fgets(author, AUTHOR_LEN, stdin); trim_newline(author);
    if (strlen(author) == 0) strcpy(author, "Unknown");

    printf("Enter number of copies: ");
    if (scanf("%d", &copies) != 1) { 
        while (getchar() != '\n'); 
        printf("Invalid number.\n"); 
        return; 
    }
    while (getchar() != '\n');

    Book *b = (Book*)malloc(sizeof(Book));
    if (!b) { perror("malloc"); return; }
    b->id = next_id();
    strncpy(b->title, title, TITLE_LEN-1); b->title[TITLE_LEN-1] = '\0';
    strncpy(b->author, author, AUTHOR_LEN-1); b->author[AUTHOR_LEN-1] = '\0';
    b->copies = copies > 0 ? copies : 0;
    b->next = head;
    head = b;

    printf("Book aduded. ID = %d\n", b->id);
}

void display_books() {
    if (!head) { printf("No books in library.\n"); return; }
    printf("ID\tCopies\tTitle (by Author)\n----------------------------------------------\n");
    Book *cur = head;
    while (cur) {
        printf("%d\t%d\t%s (by %s)\n", cur->id, cur->copies, cur->title, cur->author);
        cur = cur->next;
    }
}

Book* find_by_id(int id) {
    Book *cur = head;
    while (cur) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

void search_book() {
    int choice;
    printf("Search by: 1) ID  2) Title substring\nChoose: ");
    if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); printf("Invalid.\n"); return; }
    while (getchar()!='\n');
    if (choice == 1) {
        int id;
        printf("Enter ID: ");
        if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid.\n"); return; }
        while (getchar()!='\n');
        Book *b = find_by_id(id);
        if (!b) printf("Book with ID %d not found.\n", id);
        else printf("Found: %d - %s (by %s). Copies: %d\n", b->id, b->title, b->author, b->copies);
    } else if (choice == 2) {
        char q[TITLE_LEN];
        printf("Enter title substring: ");
        fgets(q, TITLE_LEN, stdin); trim_newline(q);
        if (strlen(q) == 0) { printf("Empty query.\n"); return; }
        Book *cur = head;
        int found = 0;
        while (cur) {
            if (strcasestr_compat(cur->title, q) != NULL) {
                printf("%d - %s (by %s). Copies: %d\n", cur->id, cur->title, cur->author, cur->copies);
                found = 1;
            }
            cur = cur->next;
        }
        if (!found) printf("No books matched '%s'\n", q);
    } else {
        printf("Invalid choice.\n");
    }
}

void issue_book() {
    int id;
    printf("Enter book ID to issue: ");
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid.\n"); return; }
    while (getchar()!='\n');
    Book *b = find_by_id(id);
    if (!b) { printf("No book with ID %d.\n", id); return; }
    if (b->copies <= 0) { printf("No copies available to issue.\n"); return; }
    b->copies--;
    printf("Issued '%s'. Remaining copies: %d\n", b->title, b->copies);
}

void return_book() {
    int id;
    printf("Enter book ID to return: ");
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid.\n"); return; }
    while (getchar()!='\n');
    Book *b = find_by_id(id);
    if (!b) { printf("No book with ID %d.\n", id); return; }
    b->copies++;
    printf("Returned '%s'. Copies now: %d\n", b->title, b->copies);
}

void delete_book() {
    int id;
    printf("Enter book ID to delete: ");
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid.\n"); return; }
    while (getchar()!='\n');
    Book *cur = head, *prev = NULL;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next;
            else head = cur->next;
            printf("Deleted book '%s' (ID %d).\n", cur->title, cur->id);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    printf("Book with ID %d not found.\n", id);
}

void free_all() {
    Book *cur = head;
    while (cur) {
        Book *nx = cur->next;
        free(cur);
        cur = nx;
    }
    head = NULL;
}

void menu() {
    printf("\n=== Library Management ===\n");
    printf("1. Add book\n");
    printf("2. Display all books\n");
    printf("3. Search book\n");
    printf("4. Issue book\n");
    printf("5. Return book\n");
    printf("6. Delete book\n");
    printf("7. Save & Exit\n");
    printf("Choose option: ");
}

int main() {
    load_books();
    int choice;
    while (1) {
        menu();
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); 
            printf("Invalid input.\n");
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: add_book(); break;
            case 2: display_books(); break;
            case 3: search_book(); break;
            case 4: issue_book(); break;
            case 5: return_book(); break;
            case 6: delete_book(); break;
            case 7:
                save_books();
                free_all();
                printf("Saved to %s. Exiting.\n", FILENAME);
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}