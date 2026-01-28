#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DATA_FILE "recipes.db"
#define MAX_RECIPES 1000
#define MAX_NAME 100
#define MAX_INGREDIENTS 500
#define MAX_INSTRUCTIONS 2000

typedef struct {
    int id;
    char name[MAX_NAME];
    char ingredients[MAX_INGREDIENTS];
    char instructions[MAX_INSTRUCTIONS];
    int time_minutes;
    char difficulty[20];
    int deleted; // 0 - active, 1 - deleted
} Recipe;

Recipe recipes[MAX_RECIPES];
int recipe_count = 0;
int next_id = 1;

/* ------------------------------------------------------------------------
   Вспомогательная функция: удаляет \n и \r
------------------------------------------------------------------------ */
void trim_newline(char* s) {
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n') s[len - 1] = '\0';
    len = strlen(s);
    if (len > 0 && s[len - 1] == '\r') s[len - 1] = '\0';
}

/* Перевод строки в нижний регистр */
void to_lower_inplace(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

/* Загрузка базы данных из файла */
void load_data() {
    FILE* f = fopen(DATA_FILE, "r");
    if (!f) return;

    char line[4096];
    Recipe tmp;

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strcmp(line, "---RECIPE---") == 0) {
            tmp.id = 0;
            tmp.name[0] = '\0';
            tmp.ingredients[0] = '\0';
            tmp.instructions[0] = '\0';
            tmp.time_minutes = 0;
            tmp.difficulty[0] = '\0';
            tmp.deleted = 0;

            while (fgets(line, sizeof(line), f)) {
                trim_newline(line);
                if (strcmp(line, "---END---") == 0) break;
                if (strncmp(line, "id:", 3) == 0) tmp.id = atoi(line + 3);
                else if (strncmp(line, "name:", 5) == 0) strncpy(tmp.name, line + 5, MAX_NAME - 1);
                else if (strncmp(line, "ingredients:", 12) == 0) strncpy(tmp.ingredients, line + 12, MAX_INGREDIENTS - 1);
                else if (strncmp(line, "instructions:", 13) == 0) strncpy(tmp.instructions, line + 13, MAX_INSTRUCTIONS - 1);
                else if (strncmp(line, "time:", 5) == 0) tmp.time_minutes = atoi(line + 5);
                else if (strncmp(line, "difficulty:", 11) == 0) strncpy(tmp.difficulty, line + 11, sizeof(tmp.difficulty) - 1);
                else if (strncmp(line, "deleted:", 8) == 0) tmp.deleted = atoi(line + 8);
            }

            if (recipe_count < MAX_RECIPES) {
                recipes[recipe_count++] = tmp;
                if (tmp.id >= next_id) next_id = tmp.id + 1;
            }
        }
    }
    fclose(f);
}

/* Сохранение базы данных в файл */
void save_data() {
    FILE* f = fopen(DATA_FILE, "w");
    if (!f) {
        printf("Error: cannot open file '%s' for writing\n", DATA_FILE);
        return;
    }

    for (int i = 0; i < recipe_count; ++i) {
        Recipe* r = &recipes[i];
        fprintf(f, "---RECIPE---\n");
        fprintf(f, "id:%d\n", r->id);
        fprintf(f, "name:%s\n", r->name);
        fprintf(f, "ingredients:%s\n", r->ingredients);
        fprintf(f, "instructions:%s\n", r->instructions);
        fprintf(f, "time:%d\n", r->time_minutes);
        fprintf(f, "difficulty:%s\n", r->difficulty);
        fprintf(f, "deleted:%d\n", r->deleted);
        fprintf(f, "---END---\n");
    }

    fclose(f);
}

/* Добавление рецепта */
void add_recipe() {
    if (recipe_count >= MAX_RECIPES) {
        printf("Database is full. Cannot add a new recipe.\n");
        return;
    }

    Recipe r;
    r.id = next_id++;
    r.deleted = 0;

    char buffer[4096];

    printf("Enter recipe name: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    trim_newline(buffer);
    strncpy(r.name, buffer, MAX_NAME - 1);

    printf("Enter ingredients (comma separated): ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    trim_newline(buffer);
    strncpy(r.ingredients, buffer, MAX_INGREDIENTS - 1);

    printf("Enter instructions: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    trim_newline(buffer);
    strncpy(r.instructions, buffer, MAX_INSTRUCTIONS - 1);

    printf("Enter cooking time (minutes): ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    r.time_minutes = atoi(buffer);

    printf("Enter difficulty (easy, medium, hard): ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    trim_newline(buffer);
    strncpy(r.difficulty, buffer, sizeof(r.difficulty) - 1);

    recipes[recipe_count++] = r;
    printf("Recipe added with ID = %d\n", r.id);
}

/* Удаление по ID */
void delete_recipe() {
    char buffer[128];
    printf("Enter recipe ID to delete: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;

    int id = atoi(buffer);

    for (int i = 0; i < recipe_count; ++i) {
        if (recipes[i].id == id && recipes[i].deleted == 0) {
            printf("Found: %s (ID=%d). Delete? (y/n): ", recipes[i].name, id);
            if (!fgets(buffer, sizeof(buffer), stdin)) return;
            if (buffer[0] == 'y' || buffer[0] == 'Y') {
                recipes[i].deleted = 1;
                printf("Recipe marked as deleted.\n");
            }
            else {
                printf("Deletion cancelled.\n");
            }
            return;
        }
    }

    printf("Recipe with ID=%d not found.\n", id);
}

/* Сортировка выбором по имени (нечувствительно к регистру) */
void selection_sort(Recipe* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            char name_j[MAX_NAME], name_min[MAX_NAME];
            strncpy(name_j, arr[j].name, MAX_NAME);
            strncpy(name_min, arr[min_idx].name, MAX_NAME);
            name_j[MAX_NAME - 1] = '\0';
            name_min[MAX_NAME - 1] = '\0';
            to_lower_inplace(name_j);
            to_lower_inplace(name_min);
            if ((arr[j].deleted < arr[min_idx].deleted) ||
                (arr[j].deleted == arr[min_idx].deleted && strcmp(name_j, name_min) < 0)) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            Recipe tmp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = tmp;
        }
    }
}

/* Вывод всех активных рецептов в алфавитном порядке */
void list_recipes_sorted() {
    if (recipe_count == 0) {
        printf("Database is empty.\n");
        return;
    }

    Recipe* copy = malloc(sizeof(Recipe) * recipe_count);
    if (!copy) {
        printf("Memory error.\n");
        return;
    }

    memcpy(copy, recipes, sizeof(Recipe) * recipe_count);
    selection_sort(copy, recipe_count);

    printf("\nRecipe list (alphabetical):\n");

    for (int i = 0; i < recipe_count; i++) {
        Recipe* r = &copy[i];
        if (r->deleted) continue;

        printf("ID: %d\nName: %s\nIngredients: %s\nTime: %d min\nDifficulty: %s\nInstructions: %s\n---\n",
            r->id, r->name, r->ingredients, r->time_minutes, r->difficulty, r->instructions);
    }

    free(copy);
}

/* Поиск по названию и нескольким ингредиентам (разделяем по пробелу или запятой) */
void search_recipes() {
    char input[512];
    printf("Enter search text (name or ingredients): ");
    if (!fgets(input, sizeof(input), stdin)) return;
    trim_newline(input);

    if (strlen(input) == 0) {
        printf("Empty search string.\n");
        return;
    }

    // Преобразуем к нижнему регистру
    char lower_input[512];
    strncpy(lower_input, input, sizeof(lower_input) - 1);
    lower_input[sizeof(lower_input) - 1] = '\0';
    to_lower_inplace(lower_input);

    // Разбиваем строку на слова (по пробелам и запятым)
    char* words[50];
    int word_count = 0;
    char* token = strtok(lower_input, " ,");
    while (token && word_count < 50) {
        words[word_count++] = token;
        token = strtok(NULL, " ,");
    }

    int found = 0;
    for (int i = 0; i < recipe_count; i++) {
        Recipe* r = &recipes[i];
        if (r->deleted) continue;

        char combined[4096];
        snprintf(combined, sizeof(combined), "%s %s", r->name, r->ingredients);
        to_lower_inplace(combined);

        int all_words_present = 1;
        for (int w = 0; w < word_count; w++) {
            if (!strstr(combined, words[w])) {
                all_words_present = 0;
                break;
            }
        }

        if (all_words_present) {
            printf("ID: %d\nName: %s\nIngredients: %s\nTime: %d min\nDifficulty: %s\nInstructions: %s\n---\n",
                r->id, r->name, r->ingredients, r->time_minutes, r->difficulty, r->instructions);
            found++;
        }
    }

    if (found == 0)
        printf("No matches found.\n");
    else
        printf("Records found: %d\n", found);
}

/* Главное меню */
int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    load_data();
    printf("=== Culinary Recipes Database ===\n");

    char choice[16];

    while (1) {
        printf("\nMenu:\n");
        printf("1 - Add recipe\n");
        printf("2 - Delete recipe (by ID)\n");
        printf("3 - Search\n");
        printf("4 - Show all recipes (alphabetical)\n");
        printf("5 - Save and exit\n");
        printf("0 - Exit without saving\n");
        printf("Choice: ");

        if (!fgets(choice, sizeof(choice), stdin)) break;
        int ch = atoi(choice);

        switch (ch) {
        case 1: add_recipe(); break;
        case 2: delete_recipe(); break;
        case 3: search_recipes(); break;
        case 4: list_recipes_sorted(); break;
        case 5: save_data(); printf("Data saved.\n"); return 0;
        case 0: printf("Exit without saving.\n"); return 0;
        default: printf("Invalid choice.\n"); break;
        }
    }

    return 0;
}
