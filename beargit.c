#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}


/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE* fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  int is_existed = 0;
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      is_existed = 1;
    } else {
      fprintf(fnewindex, "%s\n", line);
    }
  }
  
  fclose(findex);
  fclose(fnewindex); 
  if (is_existed) {
    fs_mv(".beargit/.newindex", ".beargit/.index");
  } else {
    fs_rm(".beargit/.newindex");
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    return 1;
  }
  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  int i;
  for (i = 0; msg[i] != '\0'; i++) {
    if (msg[i] == go_bears[0]) {
      int j, k, is_contained = 1;
      for (j = i, k = 0; msg[j] != '\0' && go_bears[k] != '\0'; j++, k++) {
        if (msg[j] != go_bears[k]) {
          is_contained = 0;
          break;
        }
      }
      if (is_contained) {
        return 1;
      } 
    }
  }
  return 0;
}

void next_commit_id(char* commit_id) {
  /* COMPLETE THE REST */
  int length, i;
  for (length = 0; commit_id[length] != '\0'; length++)
    ;

  if (commit_id[length-1] == '0') {
    strcpy(commit_id, "1111111");
  } else {
    for (i = length-1; i >= 0; i--) {
      if (commit_id[i] == '1') {
        commit_id[i] = '6';
        break;
      } else if (commit_id[i] == '6') {
        commit_id[i] = 'c';
        break;
      } else {
        commit_id[i] = '1';
      }
    }
  }
}

char* concat(const char *s1, const char *s2) {
  char *result = malloc(strlen(s1) +  strlen(s2) + 1);
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  /* COMPLETE THE REST */
  char* new_dir = concat(".beargit/", commit_id);
  char* new_index = concat(new_dir, "/.index");
  char* new_msg = concat(new_dir, "/.msg");
  char* new_prev = concat(new_dir, "/.prev");
  fs_mkdir(new_dir);
  
  fs_cp(".beargit/.index", new_index);
  fs_cp(".beargit/.prev", new_prev);

  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    char* new_file_name = concat(new_dir, concat("/", line));
    fs_cp(line, new_file_name);
    free(new_file_name);
  }

  fclose(findex);

  write_string_to_file(".beargit/.prev", commit_id);
  write_string_to_file(new_msg, msg);
  free(new_dir);
  free(new_index);
  free(new_msg);
  free(new_prev);  
  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  int i = 0;
  printf("Tracked files:\n\n");
  while(fgets(line, sizeof(line), findex)) {
    printf("  %s", line);
    i++;
  }
  printf("\n%d files total\n", i);
  fclose(findex);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */
int print_commit(char* line) {
  int length;
  for (length = 0; line[length] != '\0'; length++)
    ;
  if (line[length-1] == '0') {
    return 0;
  }

  char* new_dir = concat(".beargit/", line);
  char* new_prev = concat(new_dir, "/.prev");
  char* new_msg = concat(new_dir, "/.msg");

  FILE* fmsg = fopen(new_msg, "r");
  char msg[COMMIT_ID_SIZE];
  fgets(msg, COMMIT_ID_SIZE, fmsg);

  FILE* fprev = fopen(new_prev, "r");
  char prev_id[MSG_SIZE];
  fgets(prev_id, MSG_SIZE, fprev);
  
  
  printf("commit <%s>\n", line);
  printf("    <%s>\n", msg);
  printf("\n");
  print_commit(prev_id);
    
  free(new_dir);
  free(new_prev);
  free(new_msg);
  return 0;
}

int beargit_log() {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.prev", "r");
  char line[COMMIT_ID_SIZE];
  fgets(line, COMMIT_ID_SIZE, findex);
  int length;
  for (length = 0; line[length] != '\0'; length++)
    ;
  if (line[length-1] == '0') {
    fprintf(stderr, "ERROR: There are no commits!\n");
  } else {
    printf("\n");
    print_commit(line);
  }
  return 0;
}
