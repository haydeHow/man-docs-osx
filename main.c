#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define TARGET_DIR "/usr/bin"

int is_regular_file(const char *path) {
  struct stat path_stat;
  return (stat(path, &path_stat) == 0) && S_ISREG(path_stat.st_mode);
}

int main() {
  DIR *dir = opendir(TARGET_DIR);
  if (!dir) {
    perror("opendir failed");
    return 1;
  }

  struct dirent *entry;

  while ((entry = readdir(dir))) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", TARGET_DIR, entry->d_name);

    if (is_regular_file(full_path)) {
      // Prepare command to get man page, clean formatting with col -bx
      char command[512];
      snprintf(command, sizeof(command), "man %s | col -bx", entry->d_name);

      // Open pipe to read man output
      FILE *pipe = popen(command, "r");
      if (!pipe) {
        fprintf(stderr, "Failed to run man for: %s\n", entry->d_name);
        continue;
      }

      // Output file path (saved one folder up)
      char outfile[512];
      snprintf(outfile, sizeof(outfile), "../%s.man.txt", entry->d_name);
      FILE *out = fopen(outfile, "w");
      if (!out) {
        perror("fopen failed");
        pclose(pipe);
        continue;
      }

      // Write man output to file
      char buffer[1024];
      while (fgets(buffer, sizeof(buffer), pipe)) {
        fputs(buffer, out);
      }

      fclose(out);
      pclose(pipe);

      printf("Wrote man page: %s\n", outfile);
    }
  }

  closedir(dir);
  return 0;
}
