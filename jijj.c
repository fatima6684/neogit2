

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <glob.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
// wildcard ha bein " " bayad bashan
#define MAX_FILENAME_LENGTH 1000
#define MAX_COMMIT_MESSAGE_LENGTH 2000
#define MAX_LINE_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1000
#define RED_COLOR "\x1b[31m"
#define BLUE_COLOR "\x1b[34m"
#define RESET_COLOR "\x1b[0m"
#define YELLOW_COLOR "\x1b[33m"
int is_global=0;
#define debug(x) printf("%s", x);
char username1[500];
char email1[500];

void print_command(int argc, char * const argv[]);

int run_init(int argc, char * const argv[]);
int create_configs(char *username, char *email);
int find_which_branch();
int add_to_staging(char *filepath);

int run_reset(int argc, char * const argv);
int remove_from_staging(char *filepath);
int check_hooks(char *filename);
int run_commit(int argc, char * const argv[]);
int inc_last_commit_ID();
bool check_file_directory_exists(char *filepath);
int commit_staged_file(int commit_ID, char *filepath);
int track_file(char *filepath);
bool is_tracked(char *filepath);
int create_commit_file(int commit_ID, char *message,char * time);
int find_file_last_commit(char* filepath);

int run_checkout(int argc, char *const argv[]);
int find_file_last_change_before_commit(char *filepath, int commit_ID);
int checkout_file(char *filepath, int commit_ID);
char branch[300];
void print_command(int argc, char * const argv[]) {
    for (int i = 0; i < argc; i++) {
        fprintf(stdout, "%s ", argv[i]);
    }
    fprintf(stdout, "\n");
}
/*
struct FileInfo {
    char name[MAX_FILENAME_LENGTH];
    time_t modifiedTime;
};

struct FileInfo* previousFiles = NULL;
int previousFileCount = 0;

// Function to get the modification time of a file
time_t getModifiedTime(const char* filePath) {
    struct stat attr;
    stat(filePath, &attr);
    return attr.st_mtime;
}*/
int save_time(){
    FILE* file=fopen(".neogit/times","w");
    char path[100];
    DIR *dir = opendir(".");
        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            strcpy(path,entry->d_name);
            struct stat fileInfo;
            if (stat(path, &fileInfo) == -1) {
                printf("Error: Unable to get file/directory information.\n");
                return 1;
            }
            
            //fprintf(file,"%s\n", path);
            
            struct stat foo;
            time_t mtime;
            struct utimbuf new_times;

            stat (path, &foo);
            mtime = foo.st_mtime; /* seconds since the epoch */
            fprintf(file,"%s\n%s", path, ctime(&mtime));
        }
        fclose(file);
    closedir(dir);
    return 0;
}


int run_init(int argc, char * const argv[]) {
    //checking_status();
    //fprintf(stdout,"hh");
   
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;

    char tmp_cwd[1024];
    bool exists = false;
    struct dirent *entry;
    do {
        // find .neogit
        DIR *dir = opendir(".");
        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".neogit") == 0)
                exists = true;
        }
        closedir(dir);

        // update current working directory
        if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) return 1;

        // change cwd to parent
        if (strcmp(tmp_cwd, "/") != 0) {
            if (chdir("..") != 0) return 1;
        }

    } while (strcmp(tmp_cwd, "/") != 0);

    // return to the initial cwd
    if (chdir(cwd) != 0) return 1;
    FILE* globalfile=fopen("global","r");
    if (!exists) {
        char username[200];
        char email[200];
        if (mkdir(".neogit", 0755) != 0) return 1;
        fprintf(stdout,"please enter your username and email\n");
        fprintf(stdout,"username:");
        fscanf(stdin,"%s",username);
        if(email == NULL){fprintf(stdout,"error\nusername:");fscanf(stdin,"%s",username);}
        fprintf(stdout,"email:");
        fscanf(stdin,"%s",email);
        if(email == NULL){fprintf(stdout,"error\nemail:");fscanf(stdin,"%s",email);}
        fprintf(stdout,"if you want this email and user name to be global use commit neogit config –global...\n");
        return create_configs(username, email);
    } else {
        if(globalfile!=NULL){
            //printf("%d",is_global);
            char username[200];
            char email[200];
            fprintf(stdout,"please enter your username and email\n");
            fprintf(stdout,"username:");
            fscanf(stdin,"%s",username);
            if(email == NULL){fprintf(stdout,"error\nusername:");fscanf(stdin,"%s",username);}
            fprintf(stdout,"email:");
            fscanf(stdin,"%s",email);
            if(email == NULL){fprintf(stdout,"error\nemail:");fscanf(stdin,"%s",email);}
            fprintf(stdout,"if you want this email and user name to be global use commit neogit config –global...\n");
            return create_configs(username, email);
        }
        perror("neogit repository has already initialized");
    }
    return 0;
}
int finding_max(char * filepath_dir){
    
}
int create_configs(char *username, char *email) {
    //printf("**");
    int maximum=inc_last_commit_ID();
    FILE *file = fopen(".neogit/config", "w");
    if (file == NULL){printf("error"); return 1;}
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    if (maximum==1){strcpy(branch,"master");}
    else{find_which_branch();}
    fprintf(file, "username: %s\n", username);
    fprintf(file, "email: %s\n", email);
    fprintf(file, "last_commit_ID: %d\n", maximum-1);
    fprintf(file, "current_commit_ID: %d\n", maximum-1);
    //find_which_branch();
    fprintf(file, "branch: %s", branch);

    fclose(file);
    // create commits folder
    if (mkdir(".neogit/commits", 0755) != 0) return 1;

    // create files folder
    if (mkdir(".neogit/files", 0755) != 0) return 1;
    if (mkdir(".neogit/tags", 0755) != 0) return 1;
    file = fopen(".neogit/staging", "w");
    fclose(file);
    file = fopen(".neogit/tracks", "w");
    fclose(file);
     file = fopen(".neogit/max", "w");
    fclose(file);
    file = fopen(".neogit/readycommit", "w");
    fclose(file);
    file = fopen(".neogit/stagefile", "w");
    fclose(file);
    file = fopen(".neogit/now_branch", "w");
    fprintf(file,"master\n");
    fclose(file);
    file = fopen(".neogit/branches", "w");
    fprintf(file,"master\n");
    fclose(file);
     save_time();
    return 0;
}
int check_exist(int argc,char *const argv[]){
    bool fileExists = false;
        char targetFile[100] ;
        strcpy(targetFile,argv[2]); // replace "target.txt" with the name of the file you want to search for
        char currentDir[PATH_MAX];
        do {
        // Update current working directory
        
        if (getcwd(currentDir, sizeof(currentDir)) == NULL) return 1;

        // Open current directory
        DIR *dir = opendir(currentDir);
        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }

        // Look for the target file in the current directory
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strcmp(entry->d_name, targetFile) == 0) {
                fileExists = true;
                break;
            }
        }

        closedir(dir);

        // Change current directory to parent
        if (strcmp(currentDir, "/") != 0) {
            if (chdir("..") != 0) return 1;
        }

        } while (strcmp(currentDir, "/") != 0 && !fileExists);
        fopen("k.txt","w");
        if (fileExists) {
            //printf("File found in directory: %s\n", currentDir);
            
        } else {
            //printf("File not found\n");
        }
        char tmp_cwd[1024];
    bool exists = false;
    struct dirent *entry;
    do {
        // find .neogit
        DIR *dir = opendir(".");
        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".neogit") == 0)
                exists = true;
        }
        closedir(dir);

        // update current working directory
        if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) return 1;

        // change cwd to parent
        if (strcmp(tmp_cwd, "/") != 0) {
            if (chdir("..") != 0) return 1;
        }

    } while (strcmp(tmp_cwd, "/") != 0);
        //printf("J");
        //run_init(argc,argv);
        //printf("m");
}
int run_add_chand(int argc, char *const argv){
    if (argc < 3) {
        perror("please specify a file");
        return 1;
    }
    else{
        //checking directory or regular file
        DIR *dir = opendir(argv);
        if (dir == NULL) {
            //checking existing file
           if (access(argv, F_OK) == 0) {
            printf("%s exists\n", argv);
            return add_to_staging(argv);
            } else {
                printf("%s does not exist\n", argv);
            }
        }
        else{
            if (access(argv, F_OK) == 0) {
            printf("%s exists\n", argv);
            return add_to_staging(argv);
            /*struct dirent *entry;
            struct stat info;
            while ((entry = readdir(dir)) != NULL) {
                char file_path[MAX_FILENAME_LENGTH];
                strcpy(file_path,argv);
                strcat(file_path,"/");
                strcat(file_path,entry->d_name);
                add_to_staging(file_path); // Construct full path
                //printf("%s\n",file_path);
            }*/
            //
            } else {
                printf("%s does not exist\n", argv);
            }
        }
    }
    return 0;
}

int add_to_staging(char *filepath) {
    FILE *file = fopen(".neogit/staging", "r");
    FILE* file1=fopen(".neogit/time_stage","w");
    if (file == NULL){printf("error");return 0;}
    char line[MAX_LINE_LENGTH];
    //add_file(filepath);
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(filepath, line) == 0) {
            printf("this file-directory was staged \nnew file-directory staged successfully\n");return 0;}
    }
    fclose(file);
    file = fopen(".neogit/staging","a");
    if (file == NULL) {printf("error");return 1;}
    
    fprintf(file, "%s\n", filepath);
    printf("file-directory staged successfully\n");
    fclose(file);
    return 0;
}


int run_reset(int argc, char *const argv) {
    if (argc < 3) {
        perror("please specify a file");
        return 1;
    }
    else{
    int flag1=0;
    bool fileExists = false;
char targetFile[100];
strcpy(targetFile, argv); // replace "target.txt" with the name of the file you want to search for
char currentDir[PATH_MAX];

// Get current working directory
if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
    perror("Error getting current directory");
    return 1;
}

// Open current directory
DIR *dir1 = opendir(currentDir);
if (dir1 == NULL) {
    perror("Error opening current directory");
    return 1;
}

// Look for the target file in the current directory
struct dirent *entry;
while ((entry = readdir(dir1)) != NULL) {
    if (entry->d_type == DT_REG && strcmp(entry->d_name, targetFile) == 0) {
        fileExists = true;
        break;
    }
}

closedir(dir1);

if (fileExists) {
    printf("File found in directory: %s\n ", currentDir);flag1=1;}
else{printf("file doesnt exist\n");}



//directory
bool dirExists = false;
char targetDir[100];
strcpy(targetDir, argv); // replace "target" with the name of the directory you want to search for

// Open current directory
DIR *dir3 = opendir(currentDir);
if (dir3 == NULL) {
    perror("Error opening current directory");
    return 1;
}

// Look for the target directory in the current directory
//struct dirent *entry;
while ((entry = readdir(dir3)) != NULL) {
    if (entry->d_type == DT_DIR && strcmp(entry->d_name, targetDir) == 0) {
        dirExists = true;
        break;
    }
}

closedir(dir3);

if (dirExists) {
    printf("Directory exists.\n");flag1=1;
} else {
    printf("Directory does not exist.\n");
}
    // TODO: handle command in non-root directories 
    
    
    if(flag1==1){return remove_from_staging(argv);}}
}

int remove_from_staging(char *filepath) {
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    
    FILE *tmp_file = fopen(".neogit/tmp_staging", "w");
    if (tmp_file == NULL) return 1;
    FILE *unstaged_reset =fopen(".neogit/unstaged", "w");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length-1 ] = '\0';
        }

        if (strcmp(filepath, line) != 0) {fputs(line, tmp_file);fputs("\n",tmp_file);}
        else{fputs(line, unstaged_reset);fputs("\n",unstaged_reset);}
    }
    fclose(file);
    fclose(tmp_file);
    fclose(unstaged_reset);
    remove(".neogit/staging");
    rename(".neogit/tmp_staging", ".neogit/staging");
    printf("file-directory reset successfully\n");
    return 0;
}
int run_reset_undo(int argc, char * const argv[]){
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    
    FILE *tmp_file = fopen(".neogit/tmp_staging", "w");
    if (tmp_file == NULL) return 1;

    char line[MAX_LINE_LENGTH];
    //counting
    int count1=0;
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);
        //counting each line
        if (length > 0 && line[length - 1] == '\n') {
            count1++;
        }
    }
    //printf("%d\n",count1);
int count2=0;
fclose(file);
FILE *file2= fopen(".neogit/staging", "r");
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            count2++;
            line[length-1 ] = '\0';
        }
//printf("%d\n",count2);
        if (count2!=count1) {fputs(line, tmp_file);fputs("\n",tmp_file);}
    }
    fclose(file2);
    fclose(tmp_file);

    remove(".neogit/staging");
    rename(".neogit/tmp_staging", ".neogit/staging");
    printf("file-directory reset successfully\nundo successfully\n");
    return 0;
}
int run_commit(int argc, char * const argv[]) {
    FILE *file2 = fopen(".neogit/shortcut", "r");
    if (argc < 4) {
        perror("please use the correct format");
        return 1;
    }
    
    char message[MAX_MESSAGE_LENGTH];
    if(strstr(argv[2],"m")){strcpy(message, argv[3]);}
    else if(strstr(argv[2],"s")){
    int j=2;
    int i=0;
    int flag_1=0;
    char line[MAX_LINE_LENGTH];
    char temp[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int invalid_1=0;
    int count_line=0;
    while (fgets(line, sizeof(line), file2) != NULL) {
        // change line with argv4
        int length = strlen(line);
            // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {line[length - 1] = '\0';}  
        if(flag_1==1){strcpy(message,line);flag_1=0;}
        if(count_line%2==0){if (strcmp(argv[3], line) == 0) {flag_1=1;invalid_1=1;}}
            count_line++;
    }
    if(invalid_1==0){printf("invalid shortcut command\n");return 0;}
    fclose(file2);
    }
    else{printf("invalid command");return 0;}
    int d=strlen(message);if(d>72){printf("invalid command");return 0;}
    if(d==0){printf("invalid command");return 0;}
    if(message==NULL){printf("invalid command");return 0;}
    int commit_ID = inc_last_commit_ID();
    if (commit_ID == -1) return 1;
    
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    FILE * ready_to_commit=fopen(".neogit/readycommit","w");
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (!check_file_directory_exists(line)) {
            char dir_path[MAX_FILENAME_LENGTH];
            strcpy(dir_path, ".neogit/files/");
            strcat(dir_path, line);
            if (mkdir(dir_path, 0755) != 0) return 1;
        }
        int g=check_hooks(line);
        if(g==1){printf("do you want to this file be commited?(y / n):");
        char r[10];
        scanf("%s",r);
        if(r[0]=='n'){printf("this file would not be commited\n");continue;}
        }
        printf("commit %s\n", line);
        fprintf(ready_to_commit,"%s %d\n",line,find_file_last_commit(line));
        //printf("%d %s",commit_ID,line);
        commit_staged_file(commit_ID, line);
        track_file(line);
    }
    fclose(file); 
    fclose(ready_to_commit);
    // free staging
    file = fopen(".neogit/staging", "w");
    if (file == NULL) return 1;
    fclose(file);
    time_t currentTime;
    time(&currentTime);
    // Convert it to a string
    char* timeString = ctime(&currentTime);
    create_commit_file(commit_ID, message,timeString);
    fprintf(stdout,"commit = %s\n",message);
    fprintf(stdout, "commit successfully with commit ID %d\n", commit_ID);
    // Get the current time
    
    // Print the time
    printf("Current time: %s", timeString);
    return 0;
}

// returns new commit_ID
int inc_last_commit_ID() {
    FILE *file = fopen(".neogit/config", "r");
    if (file == NULL) return 1;
    
    FILE *tmp_file = fopen(".neogit/tmp_config", "w");
    if (tmp_file == NULL) return 1;

    int last_commit_ID;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &last_commit_ID);
            last_commit_ID++;
            fprintf(tmp_file, "last_commit_ID: %d\n", last_commit_ID);

        } else fprintf(tmp_file, "%s", line);
    }
    fclose(file);
    fclose(tmp_file);

    remove(".neogit/config");
    rename(".neogit/tmp_config", ".neogit/config");
    return last_commit_ID;
}
int inc_last_commit_ID_bedoon_taghirat() {
    FILE *file = fopen(".neogit/config", "r");
    if (file == NULL) return 1;
    int last_commit_ID;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &last_commit_ID);
        }
    }
    fclose(file);
    //printf("%d",last_commit_ID);
    return last_commit_ID;
}
int find_last_commit_ID() {
    FILE *file = fopen(".neogit/config", "r");
    if (file == NULL) return 1;
    int last_commit_ID;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &last_commit_ID);}
    }
    fclose(file);
    return last_commit_ID;
}
int find_file_last_commit(char* filepath) {
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/files/");
    strcat(filepath_dir, filepath);

    int max = -1;
    
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;

    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int tmp = atoi(entry->d_name);
            max = max > tmp ? max: tmp;
        }
    }
    closedir(dir);

    return max;
}

bool check_file_directory_exists(char *filepath) {
    DIR *dir = opendir(".neogit/files");
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, filepath) == 0) return true;
    }
    closedir(dir);

    return false;
}
void copy_files(const char* source_dir, const char* dest_dir) {
    DIR* dir;
    struct dirent* entry;
    struct stat file_info;
    char source_path[2000], dest_path[2000];
    int source_fd, dest_fd, n;
    char buffer[4096];

    // Open source directory
    if ((dir = opendir(source_dir)) == NULL) {
        perror("opendir()");
        exit(EXIT_FAILURE);
    }

    // Create destination directory if it doesn't exist
    mkdir(dest_dir, 0777);

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip dot (.) and double dot (..) directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build source and destination paths
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

        // Get file information
        if (stat(source_path, &file_info) < 0) {
            perror("stat()");
            exit(EXIT_FAILURE);
        }

        // Check if the entry is a regular file
        if (S_ISREG(file_info.st_mode)) {
            // Open source file
            if ((source_fd = open(source_path, O_RDONLY)) < 0) {
                perror("open()");
                exit(EXIT_FAILURE);
            }

            // Create destination file
            if ((dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, file_info.st_mode)) < 0) {
                perror("open()");
                exit(EXIT_FAILURE);
            }

            // Copy file content
            while ((n = read(source_fd, buffer, sizeof(buffer))) > 0) {
                write(dest_fd, buffer, n);
            }

            // Close file descriptors
            close(source_fd);
            close(dest_fd);
        }
    }

    // Close directory
    closedir(dir);

    printf("Directory copied successfully!\n");
}
int commit_staged_file(int commit_ID, char* filepath) {
    DIR *dir = opendir(".");
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name,filepath)==0) {
            //regular file
            FILE *read_file, *write_file;
            char read_path[MAX_FILENAME_LENGTH];
            strcpy(read_path, filepath);
            char write_path[MAX_FILENAME_LENGTH];
            strcpy(write_path, ".neogit/files/");
            strcat(write_path, filepath);
            strcat(write_path, "/");
            char tmp[10];
            sprintf(tmp, "%d", commit_ID);
            strcat(write_path, tmp);

            read_file = fopen(read_path, "r");
            if (read_file == NULL) return 1;
            write_file = fopen(write_path, "w");
            if (write_file == NULL) return 1;
            //printf("*");
            char buffer;
            buffer = fgetc(read_file);
            while(buffer != EOF) {
                fputc(buffer, write_file);
                buffer = fgetc(read_file);
            }
            fclose(read_file);
            fclose(write_file);

        }
        if (entry->d_type == DT_DIR && strcmp(entry->d_name,filepath)==0) {
            //directory
            FILE *read_file, *write_file;
            char read_path[MAX_FILENAME_LENGTH];
            strcpy(read_path, filepath);
            char write_path[MAX_FILENAME_LENGTH];
            strcpy(write_path, ".neogit/files/");
            strcat(write_path, filepath);
            strcat(write_path, "/");
            char tmp[10];
            sprintf(tmp, "%d", commit_ID);
            strcat(write_path, tmp);
            //printf("*");
            copy_files(read_path,write_path);
        }
    }
        //return 0;
    
    return 0;
}

int track_file(char *filepath) {
    //if (is_tracked(filepath)) return 0;

    FILE *file = fopen(".neogit/tracks", "a");
    //printf("&&");
    if (file == NULL) return 1;
    //printf("@");
    fprintf(file, "%s\n", filepath);
    fclose(file);
    return 0;
}

bool is_tracked(char *filepath) {
    //printf("$");
    FILE *file = fopen(".neogit/tracks", "r");
    if (file == NULL) return false;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        //printf("%s %s ",line,filepath);
        if (strcmp(line, filepath) == 0) {return true;}

    }
    fclose(file); 
    return false;
}

int create_commit_file(int commit_ID, char *message,char * timeString) {
    
    char commit_filepath[MAX_FILENAME_LENGTH];
    strcpy(commit_filepath, ".neogit/commits/");
    char tmp[10];
    sprintf(tmp, "%d", commit_ID);
    //printf("%d\n",commit_ID);
    strcat(commit_filepath, tmp);

    FILE *file = fopen(commit_filepath, "w");
    FILE *file3 = fopen(".neogit/config", "r");
    if (file3 == NULL)return 1;
    char line[MAX_LINE_LENGTH];
    int i=0;
    while (fgets(line, sizeof(line), file3) != NULL) {
        if(i==0){fprintf(file, "%s", line);}
        i++;
    }
    find_which_branch();
    fprintf(file, "branch: %s\n", branch);
    fclose(file3);
    if (file == NULL) return 1;
    fprintf(file, "message: %s\n", message);
    fprintf(file, "time: %s", timeString);
    fprintf(file, "files:\n");
    
    DIR *dir = opendir(".");
    //printf("%s\n",dir);
    //dir > output.txt
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        //printf("*%s*\n",entry->d_name);
        //inja faghat txt ghaboll mikone
        //if (entry->d_type == DT_REG && is_tracked(entry->d_name)) {
        if (is_tracked(entry->d_name)) {
            int file_last_commit_ID = find_file_last_commit(entry->d_name);
            fprintf(file, "%s %d\n", entry->d_name, file_last_commit_ID);

        }
    }
    closedir(dir); 
    fclose(file);
    FILE *read_file=fopen(".neogit/readycommit","r");
    while (fgets(line, sizeof(line), read_file) != NULL) {
        fprintf(file, "%s", line);
    }
    fclose(read_file);
    remove(".neogit/readycommit");
    
    return 0;
}



int run_checkout_commit_id(int argc, char * const argv[]) {
    if (argc < 3) return 1;
    FILE *file=fopen(".neogit/allow","w");
    
    int commit_ID = atoi(argv[3]);
    int last_id=inc_last_commit_ID_bedoon_taghirat();
    if(commit_ID<last_id){fprintf(file,"n");}
    fclose(file);
    printf("%s",argv[3]);
    DIR *dir = opendir(".");
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        //if (entry->d_type == DT_REG && is_tracked(entry->d_name)) {
        if (is_tracked(entry->d_name)) {
            checkout_file(entry->d_name, find_file_last_change_before_commit(entry->d_name, commit_ID));
        }
    }
    printf("checkout was successfull\n");
    closedir(dir);

    return 0;
}
int run_checkout_branch(int argc, char * const argv[]) {
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir,".neogit/commits");
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;
    char last_file[MAX_FILENAME_LENGTH];
    last_file[0]='0';
    last_file[1]='\0';
    int max =0;
    while((entry = readdir(dir)) != NULL) {
        //printf("%s\n",entry->d_name);
        //if (entry->d_type == DT_REG) {
            char file_path[MAX_FILENAME_LENGTH];
            strcpy(file_path,".neogit/commits/");
            strcat(file_path,entry->d_name);
            FILE * file=fopen(file_path,"r");
            //tartib file ha
            
            char line[MAX_LINE_LENGTH];
            line[0]='\0';
            char ezafi[50];
            char branch[30];
            int i=0;
            char name_file[1000];
            if(file==NULL){printf("error");return 1;}
            while (fgets(line, sizeof(line), file) != NULL) {
                //printf("%s",line);
                if(i==0){fscanf(file,"%s %s",ezafi,branch);
                if(strcmp(argv[3],branch)==0){strcpy(last_file,entry->d_name);int temp=atoi(last_file);if(temp>max){max=temp;}}}
                i++;
            }
            fclose(file);
    //}
    }
    //printf("%d",max);
    char commit_id[30];
    sprintf(commit_id,"%d",max);
    strcpy(argv[3],commit_id);
    run_checkout_commit_id(argc,argv);
    return 0;
}

int find_file_last_change_before_commit(char *filepath, int commit_ID) {
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/files/");
    strcat(filepath_dir, filepath);
    int max = 0;
    
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;

    while((entry = readdir(dir)) != NULL) {
        //if (entry->d_type == DT_REG) {
            int tmp = atoi(entry->d_name);
            if (tmp > max && tmp <= commit_ID) {
                max = tmp;
            }
        //}
    }
    closedir(dir);

    return max;
}

int checkout_file(char *filepath, int commit_ID) {
    DIR *dir = opendir(".");
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name,filepath)==0) {
            char src_file[MAX_FILENAME_LENGTH];
            strcpy(src_file, ".neogit/files/");
            strcat(src_file, filepath);
            char tmp[10];
            sprintf(tmp, "/%d", commit_ID);
            strcat(src_file, tmp);

            FILE *read_file = fopen(src_file, "r");
            if (read_file == NULL) return 1;
            FILE *write_file = fopen(filepath, "w");
            if (write_file == NULL) return 1;
            
            char line[MAX_LINE_LENGTH];

            while (fgets(line, sizeof(line), read_file) != NULL) {
                fprintf(write_file, "%s", line);
            }
            
            fclose(read_file);
            fclose(write_file);
            
        }
        if (entry->d_type == DT_DIR && strcmp(entry->d_name,filepath)==0) {
            char remove_dir[MAX_FILENAME_LENGTH];
            strcpy(remove_dir,".");
            strcat(remove_dir,filepath);
            //printf("*%s\n",remove_dir);
            if (rmdir(remove_dir) != 0) {/*printf("failed*\n");*/}
            if (mkdir(remove_dir,0775) != 0) {/*printf("failed\n");*/}
            char src_file[MAX_FILENAME_LENGTH];
            strcpy(src_file, ".neogit/files/");
            strcat(src_file, filepath);
            char tmp[10];
            sprintf(tmp, "/%d", commit_ID);
            strcat(src_file, tmp);
            char write_file[MAX_FILENAME_LENGTH];
            //strcpy(write_file,filepath);
            //FILE *read_file = fopen(src_file, "r");
            printf("%s %s\n",filepath,src_file);
            copy_files(src_file,filepath);
        }
    }
    return 0;
}
int run_wildcard(int argc,const char *argv){
     glob_t glob_result;   // structure to store matching results
    const char *pattern = "/root/*";   // specify the directory and the pattern

    int ret = glob(argv, GLOB_TILDE, NULL, &glob_result);

    if (ret == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            printf("File: %s", glob_result.gl_pathv[i]);
            run_add_chand(argc, glob_result.gl_pathv[i]);
        }
        globfree(&glob_result);   // free allocated memory
        return 0;
    }

    printf("Error matching files.\n");
    return -1;
}
int run_wildcard_reset(int argc,const char *argv){
     glob_t glob_result;   // structure to store matching results
    const char *pattern = "/root/*";   // specify the directory and the pattern

    int ret = glob(argv, GLOB_TILDE, NULL, &glob_result);

    if (ret == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            printf("File: %s", glob_result.gl_pathv[i]);
            run_reset(argc, glob_result.gl_pathv[i]);
        }
        globfree(&glob_result);   // free allocated memory
        return 0;
    }

    printf("Error matching files.\n");
    return -1;
}
int check_staged(char * sname){
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if (strcmp(sname, line) == 0) {return 1;}
    }
    fclose(file);
    return 0;
}
int run_depth(int argc,char * const argv[]){
    int t_depth=0;
    struct dirent *entry;
    DIR *dir3 = opendir("."); // Open current directory

    if (dir3 == NULL) {
        perror("Unable to open directory");
        return 1;
    }

    while ((entry = readdir(dir3)) != NULL) {
        printf("%s ", entry->d_name);
        int d=check_staged(entry->d_name);
        if(d==1){printf("satged\n");}
        else{printf("unstaged\n");}
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
             // Skip current and parent directories
            DIR *subdir;
            struct dirent *subentry;
            subdir = opendir(entry->d_name);
            
            if (subdir) {
                while ((subentry = readdir(subdir)) != NULL) {
                    printf("%s", subentry->d_name);
                    if(d==1){printf("+\n");}
                    else{printf("-\n");}

                }
                closedir(subdir);
            }
        }
    }

    closedir(dir3);
}
int run_status_2(int argc,char * const argv[]){
    FILE* file1=fopen(".neogit/times","r");
    char line[MAX_LINE_LENGTH];
    int i=0;
    int flag_3=0;
    char time[200];
    char file[MAX_FILENAME_LENGTH];
    while (fgets(line, sizeof(line), file1) != NULL) {
        int length = strlen(line);
        //printf("%s",line);
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if (i%2==0 ) {strcpy(file,line);flag_3=1;}
        if(flag_3==1 && i%2==1){strcpy(time,line);
        char path[100];
        //printf("%s",file);
        DIR *dir = opendir(".");
        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }
        struct dirent *entry;
        printf("%s",file);

        int flag_5=0;
        FILE* file4=fopen(".neogit/staging","r");
        char line2[MAX_LINE_LENGTH];
        while (fgets(line2, sizeof(line2), file4) != NULL) {
        int length = strlen(line2);
        //printf("%s",line);
        if (length > 0 && line2[length - 1] == '\n') {
            line2[length - 1] = '\0';
        }
        if(strcmp(line2,file)==0){printf(" +");flag_5=1;break;}
        }
        if(flag_5==0){printf(" -");}


        int flag_4=0;
        while ((entry = readdir(dir)) != NULL) {
            strcpy(path,entry->d_name);
            struct stat fileInfo;
            if(strcmp(path,file)==0){
                //printf("%s",file);
                 struct stat foo;
            time_t mtime;
            struct utimbuf new_times;

            stat (file, &foo);
            mtime = foo.st_mtime; 
            char time2[300];
            
            strcpy(time2,ctime(&mtime));
            int d=strlen(time2);
            time2[d-1]='\0';
            int d2=strlen(time);
            time2[d2]='\0';
             flag_4=1;
            int result=strcmp(time2,time);
            if(strcmp(time2,time)==0){printf("A\n");continue;}
            else{printf("M\n");continue;}
}
        }
        if(flag_4==0){
        printf("D\n");}
        closedir(dir);
        }
        i++;
    }
    
    return 0;
}
int run_status(int argc,char * const argv[]){
    char file_path[MAX_FILENAME_LENGTH];
    //filepath == commit path
    FILE* file1=fopen(".neogit/times","r");
    char line[MAX_LINE_LENGTH];
    int i=0;
    int flag_3=0;
    char time[200];
    char file[MAX_FILENAME_LENGTH];
    while (fgets(line, sizeof(line), file1) != NULL) {
        int length = strlen(line);
        //printf("%s",line);
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if (i%2==0 ) {strcpy(file,line);flag_3=1;}
        if(flag_3==1 && i%2==1){strcpy(time,line);
        
        strcpy(file_path,".neogit/commits/");
        int last_commit=inc_last_commit_ID_bedoon_taghirat();
        char commit[50];
        sprintf(commit,"%d",last_commit);
        //printf(" %s ",commit);
        strcat(file_path,commit);
        FILE *file2=fopen(file_path,"r");
        //printf("%s ",file_path);
        char line2[MAX_LINE_LENGTH];
        int j=0;
        char ezafi[500];
        char time2[1000];
        int flag_4=0;

        int flag_5=0;
        FILE* file4=fopen(".neogit/staging","r");
        char sign[10];
        char line3[MAX_LINE_LENGTH];
        while (fgets(line3, sizeof(line3), file4) != NULL) {
        int length = strlen(line3);
        
        if (length > 0 && line3[length - 1] == '\n') {
            line3[length - 1] = '\0';
        }
        //printf("%s %s*",line3,file);
        if(strcmp(line3,file)==0){strcpy(sign,"+");flag_5=1;break;}
        }
        if(flag_5==0){strcpy(sign,"-");}
        fclose(file4);
        char line4[MAX_LINE_LENGTH];
        while (fgets(line4, sizeof(line4), file2) != NULL) {
            //printf("%s",line4);
            int length = strlen(line4);
            //printf("%s",line);
            if (length > 0 && line4[length - 1] == '\n') {
                int count=0;
                if(j>4){
                while(1){
                    //printf("%c\n",line4[count]);
                    if(line4[count]==' '){line4[count]='\0';break;}
                    count++;}
                }
                else{
                    if (length > 0 && line4[length - 1] == '\n') {
                    line4[length - 1] = '\0';}
                }
                //printf("*%s*",line4);
            }
            if(j==3){
                //printf("%s",line4);
                //joda kardan tarikh
                //fscanf(file2,"%s %s",ezafi,time2);
                int i, j = 0;

                // Skip any leading whitespace
                while (line4[j] == ' ') {
                    j++;
                }

                // Find the first space after the first word
                for (i = j; i < strlen(line4); i++) {
                    if (line4[i] == ' ') {
                        break;
                    }
                }
                j--;
                // Shift the remaining characters to the left
                for (; i <= strlen(line4); i++) {
                    line4[j++] = line4[i];
                }
                int d=strlen(line4);
                line4[d-1]='\0';
                strcpy(time2,line4);
                }
            if(j>4){
                //printf("#");
                //printf(" %s %s ",file,line4);
                    if(strcmp(file,line4)==0){
                    //printf("*");
                    flag_4=1;
                    //comparing times
                    struct stat foo;
                    time_t mtime;
                    struct utimbuf new_times;

                    stat (file, &foo);
                    mtime = foo.st_mtime; 
                    char time[300];
                    strcpy(time,ctime(&mtime));
                    int d2=strlen(time);
                    time2[d2]='\0';
                    int result=strcmp(time2,time);
                    if(d2<2){printf("%s %s",file,sign);printf("D\n");continue;}
                    else if(strcmp(time2,time)>=0){printf("%s %s",file,sign);printf("A\n");continue;}
                    else{printf("%s %s",file,sign);printf("M\n");continue;}
                }
            }
            j++;
        }
        //if(flag_4==0){printf("%s",file);printf("D\n");}
        }
        i++;
    }
    fclose(file1);

    //taghir file bedon commit

    file1=fopen(".neogit/times","r");
    char line5[MAX_LINE_LENGTH];
    i=0;
    flag_3=0;
    time[200];
    file[MAX_FILENAME_LENGTH];
    while (fgets(line5, sizeof(line5), file1) != NULL) {
        int length = strlen(line5);
        //printf("%s",line5);
        if (length > 0 && line5[length - 1] == '\n') {
            line5[length - 1] = '\0';
        }
        if (i%2==0 ) {strcpy(file,line5);flag_3=1;}
        if(flag_3==1 && i%2==1){strcpy(time,line5);
            struct stat foo;
            time_t mtime;
            struct utimbuf new_times;
            
            stat (file, &foo);
            mtime = foo.st_mtime; 
            char time2[300];
            strcpy(time2,ctime(&mtime));
            int d2=strlen(time);
            time2[d2]='\0';
            if(strcmp(time2,time)!=0){
                //not to print two time a file

                int flag_print=0;
                FILE *file5=fopen(file_path,"r");
                int j=0;
                while (fgets(line5, sizeof(line5), file5) != NULL) {
                int length = strlen(line5);
                if (length > 0 && line5[length - 1] == '\n') {
                    int count=0;
                    if(j>4){
                    while(1){
                        if(line5[count]==' '){line5[count]='\0';break;}
                        count++;}
                    }
                    else{
                        if (length > 0 && line5[length - 1] == '\n') {
                        line5[length - 1] = '\0';}
                    }
                }
                if(j>4){
                        if(strcmp(file,line5)==0){
                        flag_print=1;
                    }}
                j++;
                }
                if(flag_print==0){printf("%s -",file);printf("M\n");}
            }
        }
    i++;
    }
    fclose(file1);

    //deleted file
    
    FILE *file5=fopen(file_path,"r");
    int j=0;
    while (fgets(line5, sizeof(line5), file5) != NULL) {
        int length = strlen(line5);
        //printf("%s %s",line5,file);
        if (length > 0 && line5[length - 1] == '\n') {
            int count=0;
            if(j>4){
            while(1){
                if(line5[count]==' '){line5[count]='\0';break;}
                count++;}
                }
                else{
                if (length > 0 && line5[length - 1] == '\n') {
                    line5[length - 1] = '\0';}
            }
        }
        if(j>4){
        //if(strcmp(file,line5)==0){
            //serach in file times

            file1=fopen(".neogit/times","r");
            char line6[MAX_LINE_LENGTH];
            i=0;
            flag_3=0;
            time[200];
            char file_4[MAX_FILENAME_LENGTH];
            int flag_delete=0;
            while (fgets(line6, sizeof(line6), file1) != NULL) {
                int length = strlen(line6);
                //printf("%s %s",line5,line6);
                if (length > 0 && line6[length - 1] == '\n') {
                    line6[length - 1] = '\0';
                }
                if(strcmp(line5,line6)==0){flag_delete=1;}}
                if(flag_delete==0){
                    // remove '\n'
                    int d= strlen(line5);
                    line5[d-1]='\0';
                    printf("%s -D\n",line5);}
                fclose(file1);
                    }
                j++;
                }
    return 0;
}
int run_redo(int argc,char * const argv[]){
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    
    FILE *tmp_file = fopen(".neogit/tmp_staging", "w");
    if (tmp_file == NULL) return 1;
    FILE *unstaged_reset =fopen(".neogit/unstaged", "r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length-1 ] = '\0';
        }

        fputs(line, tmp_file);fputs("\n",tmp_file);
    }
    //adding unstaged
    while (fgets(line, sizeof(line), unstaged_reset) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length-1 ] = '\0';
        }

        fputs(line, tmp_file);fputs("\n",tmp_file);
    }
    fclose(file);
    fclose(tmp_file);
    fclose(unstaged_reset);
    remove(".neogit/staging");
    remove(".neogit/unstaged");
    rename(".neogit/tmp_staging", ".neogit/staging");
    printf("file-directory reset successfully\n");
    return 0;
}
int run_alias(int argc,char * const argv[]){
    FILE *file = fopen(".neogit/alias", "a+");
    if (file == NULL) {
        //printf("Error opening file.\n");
        return 1;
    }
    strtok(argv[3], "\n"); // Remove the newline character from argv[3]4

    if (strncmp(argv[3], "git ", 4) != 0) {
        printf("Error: '%s' is not a valid git command.\n", argv[3]);
        fclose(file);
        return 1;
    }
    int dastoor_sahih=0;
    if(strstr(argv[3],"add") ){dastoor_sahih=1;}
    if(strstr(argv[3],"reset") ){dastoor_sahih=1;}
    if(strstr(argv[3],"config") ){dastoor_sahih=1;}
    if(strstr(argv[3],"checkout") ){dastoor_sahih=1;}
    if(strstr(argv[3],"log") ){dastoor_sahih=1;}
    if(strstr(argv[3],"status") ){dastoor_sahih=1;}
    if(strstr(argv[3],"commit") ){dastoor_sahih=1;}
    if(strstr(argv[3],"replace") ){dastoor_sahih=1;}
    if(strstr(argv[3],"set") ){dastoor_sahih=1;}
    if(strstr(argv[3],"remove") ){dastoor_sahih=1;}
    if(strstr(argv[3],"init") ){dastoor_sahih=1;}
    if(dastoor_sahih==0){
        printf("Error: '%s' is not a valid git command.\n", argv[3]);
        fclose(file);
        return 1;
    }
    char aliasName[300];
    char aliasName_s[300];
    char command[600];
    strcpy(aliasName,argv[2]);
    int d2=strlen(aliasName);
    int j=0;
    int flagy=0;
    for (int i=0;i<d2;i++){
        if(flagy==1){aliasName_s[j]=aliasName[i];j++;}
        if(aliasName[i]=='.'){flagy=1;}
        if(aliasName[i]==' '){break;}
    }
    aliasName_s[j]='\0';
    fprintf(file, "%s\n%s\n", aliasName_s, argv[3]);
    printf("Alias '%s' saved successfully\n", aliasName);
    fclose(file);
    return 0;
}
int run_check_alias(int argc,char * argv[]){
    FILE *file = fopen(".neogit/alias", "r");
    int flag_1=0;
    if (file == NULL) {
        //printf("Error opening file.\n");
        return 1;
    }
    int j=2;
    int i=0;
    char line[MAX_LINE_LENGTH];
    char temp[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int count_line=0;
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);
        // change line with argv
        if(flag_1==1){
            char *token;
            
            // get the first token
            token = strtok(line, " ");
            // walk through other tokens
            while (token != NULL) {
                strcpy(temp[j], token); 
                //printf("%s\n", token);
                
                token = strtok(NULL, " ");
                i++;
                if(i==1){strcpy(argv[1],token);}
                if(i==2){strcpy(argv[2],token);}
            }

        }
        if(count_line%2==0){
            int length = strlen(line);
        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {line[length - 1] = '\0';}    
        if (strcmp(argv[1], line) == 0) {flag_1=1;}}
        count_line++;
    }
    if(flag_1==1){
    printf("new command:");
    print_command(argc, argv);}
    fclose(file);
    return 0;
    //return just_alias(j+1,&temp);
}
int run_commit_set(int argc,char * const argv[]){
    FILE *file = fopen(".neogit/shortcut", "a+");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    if(argc<5){printf("invalid command");return 0;}
    fprintf(file, "%s\n%s\n", argv[5], argv[3]);
    printf("command sets successfully\n");
    return 0;
}

int run_commit_replace(int argc,char * const argv[]){
    FILE *file = fopen(".neogit/shortcut", "r");
    if (file == NULL) return 1;
    int flag_1=0;
    int invalid_1=0;
    FILE *tmp_file = fopen(".neogit/tmp_shortcut", "w");
    if (tmp_file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length-1 ] = '\0';
        }
        if(flag_1==1){flag_1=0;fputs(argv[3], tmp_file);fputs("\n",tmp_file);continue;}
        if (strcmp(argv[5], line) == 0) {invalid_1=1;flag_1=1;}
        fputs(line, tmp_file);fputs("\n",tmp_file);
    }
    if(invalid_1==0){printf("invalid command");return 0;}
    fclose(file);
    fclose(tmp_file);
    remove(".neogit/shortcut");
    rename(".neogit/tmp_shortcut", ".neogit/shortcut");
    printf("command replace successfully\n");
    return 0;
}

int run_commit_remove(int argc,char * const argv[]){
    FILE *file = fopen(".neogit/shortcut", "r");
    if (file == NULL) return 1;
    int flag_1=0;
    int invalid_1=0;
    FILE *tmp_file = fopen(".neogit/tmp_shortcut", "w");
    if (tmp_file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length-1 ] = '\0';
        }
        if(flag_1==1){flag_1=0;continue;}
        if (strcmp(argv[3], line) == 0) {invalid_1=1;flag_1=1;continue;}
        fputs(line, tmp_file);fputs("\n",tmp_file);
    }
    if(invalid_1==0){printf("invalid command\n");return 0;}
    fclose(file);
    fclose(tmp_file);
    remove(".neogit/shortcut");
    rename(".neogit/tmp_shortcut", ".neogit/shortcut");
    printf("command remove successfully\n");
    return 0;
}
int run_log(){
     char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int sum_file=0;
    char number[10];
    for(int i=max;i>0;i--){
        printf("ID number : %d\n",i);
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
        printf("\n");
    }
    printf("count of all files : %d\n",sum_file);
    return 0;
}
int run_log_chand(char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int kam = argv[3][0] - '0';
    int ezafe=max-kam;
    int sum_file=0;
    char number[10];
    for(int i=max;i>ezafe;i--){
        printf("ID number : %d\n",i);
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
        printf("\n");
    }
    printf("count of all files : %d\n",sum_file);
    return 0;
}

int run_log_branch(int argc,char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int sum_file=0;
    char number[10];
    for(int i=max;i>0;i--){
        int flag_2=0;
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file1=fopen(filepath_dir,"r");
        int j=0;
        while (fgets(line, sizeof(line), file1) != NULL) {
        if(j==1){
        char* token = strtok(line,": ");
        for(int t=0;t<1;t++) {
            token = strtok(NULL, ": ");
            }
            int d = strlen(token);
            token[d-1] = '\0';
            if(strcmp(token,argv[3])==0){flag_2=1;}
            }
        j++;}
        if(flag_2==1){
        fclose(file1);
        printf("ID number : %d\n",i);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        printf("\n");}
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
    }
    printf("count of all files : %d\n",sum_file);
    if(sum_file ==0){printf("no commits find in this branch\n");}
    return 0;
}

int run_log_author(int argc,char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int sum_file=0;
    char number[10];
    for(int i=max;i>0;i--){
        int flag_2=0;
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file1=fopen(filepath_dir,"r");
        int j=0;
        while (fgets(line, sizeof(line), file1) != NULL) {
        if(j==0){
        char* token = strtok(line,": ");
        for(int t=0;t<1;t++) {
            token = strtok(NULL, ": ");
            }
            int d = strlen(token);
            token[d-1] = '\0';
            if(strcmp(token,argv[3])==0){flag_2=1;}
            }
        j++;}
        if(flag_2==1){
        fclose(file1);
        printf("ID number : %d\n",i);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        printf("\n");}
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
    }
    printf("count of all files : %d\n",sum_file);
    if(sum_file ==0){printf("no commits find for this author\n");}
    return 0;}

int run_log_since(int argc,char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int sum_file=0;
    char number[10];
    for(int i=max;i>0;i--){
        int flag_2=0;
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file1=fopen(filepath_dir,"r");
        int j=0;
        while (fgets(line, sizeof(line), file1) != NULL) {
        if(j==3){
        char tokenize_time[200];
        tokenize_time[0]='\0';
        char* token = strtok(line," ");
        for(int t=0;t<5;t++) {
            token = strtok(NULL, " ");
            strcat(tokenize_time,token);
            strcat(tokenize_time," ");
            }
            int d=strlen(tokenize_time);
            tokenize_time[d-2]='\0';
            //printf("%s\n",tokenize_time);
            char tokenize_time_2[200];
            tokenize_time_2[0]='\0';
            for(int t=3;t<8;t++) {
            token = strtok(NULL, " ");
            strcat(tokenize_time_2,argv[t]);
            strcat(tokenize_time_2," ");
            }
            tokenize_time_2[d-2]='\0';
            //printf("%s\n",tokenize_time_2);
            // Calculate the difference between the two times in seconds
            //double difference = difftime();
            int result=strcmp(tokenize_time,tokenize_time_2);
            //printf("%d\n",result);
            if (result>=0) {
                flag_2=1;
            }
            }
        j++;}
        if(flag_2==1){
        fclose(file1);
        printf("ID number : %d\n",i);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        printf("\n");}
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
    }
    printf("count of all files : %d\n",sum_file);
    if(sum_file ==0){printf("no commits find for this author\n");}
    return 0;
}

int run_log_before(int argc,char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int sum_file=0;
    char number[10];
    for(int i=max;i>0;i--){
        int flag_2=0;
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file1=fopen(filepath_dir,"r");
        int j=0;
        while (fgets(line, sizeof(line), file1) != NULL) {
        if(j==3){
        char tokenize_time[200];
        tokenize_time[0]='\0';
        char* token = strtok(line," ");
        for(int t=0;t<5;t++) {
            token = strtok(NULL, " ");
            strcat(tokenize_time,token);
            strcat(tokenize_time," ");
            }
            int d=strlen(tokenize_time);
            tokenize_time[d-2]='\0';
            //printf("%s\n",tokenize_time);
            char tokenize_time_2[200];
            tokenize_time_2[0]='\0';
            for(int t=3;t<8;t++) {
            token = strtok(NULL, " ");
            strcat(tokenize_time_2,argv[t]);
            strcat(tokenize_time_2," ");
            }
            tokenize_time_2[d-2]='\0';
            //printf("%s\n",tokenize_time_2);
            // Calculate the difference between the two times in seconds
            //double difference = difftime();
            int result=strcmp(tokenize_time,tokenize_time_2);
            //printf("%d\n",result);
            if (result<=0) {
                flag_2=1;
            }
            }
        j++;}
        if(flag_2==1){
        fclose(file1);
        printf("ID number : %d\n",i);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        printf("\n");}
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
    }
    printf("count of all files : %d\n",sum_file);
    if(sum_file ==0){printf("no commits find for this author\n");}
    return 0;
}

int run_log_search(int argc,char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/commits/");
    FILE * find_last=fopen(".neogit/config","r");
    //strcat(filepath_dir, filepath);
    int max=0;
    int k=0;
    char ezafi[50];
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), find_last) != NULL) {
        int length = strlen(line);
        if(k==1){fscanf(find_last, "%s %d",ezafi, &max);break;}
        k++;
    }
    fclose(find_last);
    int sum_file=0;
    char number[10];
    for(int i=max;i>0;i--){
        int flag_2=0;
        sprintf(number, "%d", i);  // convert the integer to a string
        strcat(filepath_dir, number);
        printf("%s\n",filepath_dir);
        FILE * file1=fopen(filepath_dir,"r");
        int j=0;
        while (fgets(line, sizeof(line), file1) != NULL) {
        if(j==2){
        char* token = strtok(line,": ");
        for(int t=0;t<1;t++) {
            token = strtok(NULL, ": ");
            }
            int d = strlen(token);
            token[d-1] = '\0';
            if(strcmp(token,argv[3])==0){flag_2=1;}
            }
        j++;}
        if(flag_2==1){
        fclose(file1);
        printf("ID number : %d\n",i);
        FILE * file=fopen(filepath_dir,"r");
        if (file == NULL) return 1;
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);sum_file++;}
        printf("\n");
        fclose(file);
        sum_file-=5;
        printf("\n");}
        filepath_dir[strlen(filepath_dir) - strlen(number)] = '\0';
    }
    printf("count of all files : %d\n",sum_file);
    if(sum_file ==0){printf("no commits find for this author\n");}
    return 0;
    }
int find_which_branch(){
    FILE * file=fopen(".neogit/now_branch","r");
    fscanf(file,"%s",branch);
    fclose(file);
    return 0;
}
int run_branch(int argc,char * const argv[]){
    FILE * check_exist_branch=fopen(".neogit/branches","r");
        char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line),check_exist_branch) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if (strcmp(line, argv[2]) == 0) {printf("the branch exists\n");return 1;}
    }
    fclose(check_exist_branch);
    FILE * file=fopen(".neogit/branches","a");
    fprintf(file,"%s\n",argv[2]);
    int int_last_commit=inc_last_commit_ID();
    char last_commit[30];
    sprintf(last_commit,"%d",int_last_commit);
    char file_path_write[MAX_FILENAME_LENGTH];
    strcpy(file_path_write,".neogit/commits/");
    strcat(file_path_write,last_commit);
    char file_path_read[MAX_FILENAME_LENGTH];
    strcpy(file_path_read,".neogit/commits/");
    sprintf(last_commit,"%d",int_last_commit-1);
    strcat(file_path_read,last_commit);
    FILE* read_file = fopen(file_path_read, "r");
    if (read_file == NULL) return 1;

    FILE *write_file = fopen(file_path_write, "w");
    if (write_file == NULL) return 1;
    printf("%s %s",file_path_read,file_path_write);
    char line1[MAX_LINE_LENGTH];
    while (fgets(line1, sizeof(line1), read_file) != NULL) {
        if(strstr(line1,"branch")){fprintf(write_file,"branch: %s\n",argv[2]);}
        else{fprintf(write_file,"%s",line1);}
    }
    printf("branch added successfully\n");
    fclose(file);
    return 0;
}
int run_print_all_branches(){
    FILE * file=fopen(".neogit/branches","r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);
    }
    fclose(file);
    return 0;
}
int run_grep(int argc,char * const argv[]){
    char read_path[MAX_FILENAME_LENGTH];
    if(strstr(argv[2],"f")==0){printf("invalid command\n");return 0;}
    if(strstr(argv[4],"p")==0){printf("invalid command\n");return 0;}
    strcpy(read_path, ".neogit/files/");
    //strcat(read_path, filepath);
    strcat(read_path, "/");
    strcat(read_path, argv[3]);
    strcat(read_path, "/");
    int number_of_commit;
    //printf("%d",argc);
    if(argc<8){number_of_commit=find_file_last_commit(argv[3]);}
    else if(strstr(argv[6],"c")){number_of_commit=atoi(argv[7]);}
    else{printf("invalid command\n");return 0;}
    char tmp[20];
    //printf("%d\n",number_of_commit);
    sprintf(tmp,"%d",number_of_commit);
    strcat(read_path, tmp);
    FILE* file=fopen(read_path,"r");
    int namayesh_number_of_lines=0;
    if(argc == 9){if(strstr(argv[8],"n")){namayesh_number_of_lines=1;}}
    if(argc == 7){if(strstr(argv[6],"n")){namayesh_number_of_lines=1;}}
    printf("%d",namayesh_number_of_lines);
    char line[MAX_LINE_LENGTH];
    int i=1;
    while (fgets(line, sizeof(line), file) != NULL) {
        
        if(strstr(line,argv[5])){
            
            if(namayesh_number_of_lines==1){printf(YELLOW_COLOR "number of line : %d  "RESET_COLOR,i);}
            //for color
            char word[MAX_LINE_LENGTH];
            int j=0;
            int k=0;
            while(1){  
                word[k]=line[j];
                k++;
                j++;
                if(line[j]==' ' || line[j]=='\n'){
                    word[k]='\0';
                    if(strstr(word,argv[5])){printf(RED_COLOR "%s" RESET_COLOR,word);}
                    else{printf("%s",word);}
                    if(line[j]=='\n'){break;}
                    k=0;}
                }
            printf("\n");}
        i++;
    }
    return 0;
}
int run_tags(int argc,char * const argv[]){
    DIR *dir;
    struct dirent *entry;
    char path[100] = ".neogit/tags/";

    // Open the directory
    dir = opendir(path);

    if (dir == NULL) {
        perror("Unable to open directory");
        return 1;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        if(strcmp(argv[3],entry->d_name)==0){
            char file_path_2[MAX_FILENAME_LENGTH];
            strcpy(file_path_2,".neogit/tags/");
            strcat(file_path_2,entry->d_name);
            FILE* file3=fopen(file_path_2,"r");
            int i=0;
            char line[MAX_LINE_LENGTH];
            while (fgets(line, sizeof(line), file3) != NULL) {
                i++;if(i==4){
                    char c[50];
                    fscanf(file3,"%s",c);
                    if(c[0]=='n'){printf("this tag exists you can not overwrite\n");return 0;}
                    else{printf("this tag exists you can overwrite\n");}}
               }}
    }
    closedir(dir);
    char file_path_write[MAX_FILENAME_LENGTH];
    strcpy(file_path_write,".neogit/tags/");
    strcat(file_path_write,argv[3]);
    //printf("%s",file_path_write);
    FILE * file1=fopen(file_path_write,"w");
    char commit_id[100];
    if(strstr(argv[6],"c")){strcpy(commit_id,argv[7]);}
    else{int id=inc_last_commit_ID_bedoon_taghirat();sprintf(commit_id,"%d",id);}

    fprintf(file1,"tag_name: %s\n",argv[3]);
    fprintf(file1,"Commit-id: %s\n",commit_id);
    FILE *file3 = fopen(".neogit/config", "r");
    if (file3 == NULL)return 1;
    char line[MAX_LINE_LENGTH];
    int i=0;
    while (fgets(line, sizeof(line), file3) != NULL) {
        if(i==0){fprintf(file1, "%s", line);}
        i++;
    }
    if(strstr(argv[4],"m")){fprintf(file1,"message: %s\n",argv[5]);}
    if(strstr(argv[8],"f")){fprintf(file1,"allowed to overwrite\n");}
    else{fprintf(file1,"not allowed to overwrite\n");}
    fclose(file3);
    fclose(file1);
    return 0;
}
int run_tags_show(int argc,char * const argv[]){
    DIR *dir;
    struct dirent *entry;
    char path[100] = ".neogit/tags/";

    // Open the directory
    dir = opendir(path);

    if (dir == NULL) {
        perror("Unable to open directory");
        return 1;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        break;
    }
    char file_path[MAX_FILENAME_LENGTH];
    strcpy(file_path,".neogit/tags/");
    strcat(file_path,entry->d_name);
    FILE * file=fopen(file_path,"r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);
    }
    return 0;
}
int compare(const void *a, const void *b) {
    return strcmp (*(const char **)a, *(const char **)b);
}
int run_print_all_tags(int argc,char * const argv[]){
    DIR *dir;
    struct dirent *ent;
    int num = 0;
    char **fileList;
    dir = opendir(".neogit/tags/");
    if (dir == NULL) {
        perror("Unable to open directory");
        return 0;
    }
    // Count the number of files in the directory
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG)
            num++;
    }
    fileList = (char**) malloc(num * sizeof(char*));

    // Reset the directory stream
    rewinddir(dir);

    // Store the file names in the directory
    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            fileList[i] = strdup(ent->d_name);
            i++;
        }
    }

    // Sort the file names
    qsort(fileList, num, sizeof(const char*), compare);

    // Print the sorted file names
    for (int i=0; i< num; i++) {
        printf("%s\n", fileList[i]);
        free(fileList[i]);
    }

    free(fileList);
    closedir(dir);
    return 0;
}
int remove_null_lines(char *filename, int start_line, int last_line){
    // Open the input file
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        printf("Unable to open the file.\n");
        return 0;
    }

    // Create a temporary file to store the modified contents
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Unable to create a temporary file.\n");
        fclose(input);
        return 0;
    }

    char buffer[100];
    int i=1;
    // Read line by line from the input file
    while (fgets(buffer, sizeof(buffer), input)) {
        if(i>=start_line){
        // Remove any newline character at the end of the line
        buffer[strcspn(buffer, "\n")] = '\0';

        // Check if the line is null or empty
        if (strlen(buffer) > 0) {
            // Write the non-null line to the temporary file
            fputs(buffer, temp);
            fputc('\n', temp);
        }}
        if(i>last_line){break;}
        //printf("%d %d\n",last_line,i);
        i++;
    }

    // Close the files
    fclose(input);
    fclose(temp);
    // Remove the input file
    remove(filename);
    // Rename the temporary file to the original filename
    rename("temp.txt", filename);
}
int run_compare(char *filename1,char *filename2){
    //printf("%s %s",filename1,filename2);
    int flag_not_same=0;
    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "r");
     char line1[MAX_LINE_LENGTH];
     char line2[MAX_LINE_LENGTH];
     int i=1;
    while (fgets(line1, MAX_LINE_LENGTH, file1) != NULL && fgets(line2, MAX_LINE_LENGTH, file2) != NULL) {
        i++;
        if (strcmp(line1, line2) != 0) {
            flag_not_same=1;
            printf(RED_COLOR "«««««\n"RESET_COLOR);
            printf(RED_COLOR"%s-%d\n"RESET_COLOR,filename1,i);
            printf(RED_COLOR"%s"RESET_COLOR, line1);
            printf(BLUE_COLOR"%s-%d\n"RESET_COLOR,filename2,i);
            printf(BLUE_COLOR"%s"RESET_COLOR, line2);
            printf(BLUE_COLOR"»»»»»\n"RESET_COLOR);
        }
    }

    if (fgets(line1, MAX_LINE_LENGTH, file1) != NULL) {
        printf(RED_COLOR"%s has additional lines:\n"RESET_COLOR,filename1);
        printf(RED_COLOR"%s-%d\n"RESET_COLOR,filename1,i+1);
        printf(RED_COLOR"%s"RESET_COLOR,line1);
        i++;
    } else if (fgets(line2, MAX_LINE_LENGTH, file2) != NULL) {
        printf(BLUE_COLOR"%s has additional lines:\n"RESET_COLOR,filename2);
        printf(BLUE_COLOR"%s-%d\n"RESET_COLOR,filename2,i+1);
        printf(BLUE_COLOR"%s"RESET_COLOR,line2);
        i++;
    }

    fclose(file1);
    fclose(file2);
    //for merge
    FILE * file3=fopen(".neogit/merge","w");
    if(flag_not_same==0){printf("texts are same\n");}
    if(flag_not_same==1){fprintf(file3,"n");return 1;}
    if(flag_not_same==0){fprintf(file3,"y");}
    //printf("%d",flag_not_same);
    fclose(file3);
    return 0;
}
int remove_null_lines_diff(char *filename, int start_line, int last_line,int number){
    // Open the input file
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        printf("Unable to open the file.\n");
        return 0;
    }
    //save in 2 different files
    /*char file_path[MAX_FILENAME_LENGTH];
    strcpy(file_path,"temp.txt");
    char num[20];
    sprintf(num,"%d",number);*/
    // Create a temporary file to store the modified contents
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Unable to create a temporary file.\n");
        fclose(input);
        return 0;
    }

    char buffer[100];
    int i=1;
    // Read line by line from the input file
    while (fgets(buffer, sizeof(buffer), input)) {
        if(i>=start_line){
        // Remove any newline character at the end of the line
        buffer[strcspn(buffer, "\n")] = '\0';

        // Check if the line is null or empty
        if (strlen(buffer) > 0) {
            // Write the non-null line to the temporary file
            fputs(buffer, temp);
            fputc('\n', temp);
        }}
        if(i>last_line){break;}
        //printf("%d %d\n",last_line,i);
        i++;
    }

    // Close the files
    fclose(input);
    fclose(temp);
    // Remove the input file
    remove(filename);
    // Rename the temporary file to the original filename
    rename("temp.txt", filename);
}
int run_diff(int argc,char * const argv[]){
    if(argc==11){
    int start_line=atoi(argv[6]);
    int last_line=atoi(argv[7]);
    remove_null_lines_diff(argv[3],start_line,last_line,1);
     start_line=atoi(argv[9]);
     last_line=atoi(argv[10]);
    remove_null_lines_diff(argv[4],start_line,last_line,2);
    }else if (argc==8){
        if(strstr(argv[5],"1")==0){
        int start_line=atoi(argv[6]);
        int last_line=atoi(argv[7]);
        remove_null_lines_diff(argv[3],start_line,last_line,1);
        remove_null_lines_diff(argv[4],1,500,2);
        }
        else{
        int start_line=atoi(argv[6]);
        int last_line=atoi(argv[7]);
        remove_null_lines_diff(argv[3],1,500,1);
        remove_null_lines_diff(argv[4],start_line,last_line,2);
        }
    }
    else{
        remove_null_lines_diff(argv[3],1,500,1);
        remove_null_lines_diff(argv[4],1,500,2);
    }
    run_compare(argv[3],argv[4]);
}
int run_diff_merge(char *file1,char *file2,char* num1,char * num2){
    printf("%s\n",file1);
    char file_path1[MAX_FILENAME_LENGTH];
    char file_path2[MAX_FILENAME_LENGTH];
    strcpy(file_path1,".neogit/files/");
    strcpy(file_path2,".neogit/files/");
    strcat(file_path1,file1);
    strcat(file_path2,file2);
    strcat(file_path1,"/");
    strcat(file_path2,"/");
    strcat(file_path1,num1);
    strcat(file_path2,num2);
    remove_null_lines(file_path1,1,500);
    remove_null_lines(file_path2,1,500);
    int d=run_compare(file_path1,file_path2);
    //d=1 not allowed to merge
    if(d==1){return 1;}
    return 0;
}
int merge(char *file_path_read_1,char *file_path_read_2){
    char commit_filepath[MAX_FILENAME_LENGTH];
    strcpy(commit_filepath, ".neogit/commits/");
    char tmp[10];
    int commit_ID=inc_last_commit_ID();
    sprintf(tmp, "%d", commit_ID);
    //printf("%d\n",commit_ID);
    strcat(commit_filepath, tmp);

    FILE *file = fopen(commit_filepath, "w");
    //username
    FILE *file2 = fopen(".neogit/config", "r");
    if (file2 == NULL)return 1;
    char line[MAX_LINE_LENGTH];
    int i=0;
    while (fgets(line, sizeof(line), file2) != NULL) {
        if(i==0){fprintf(file, "%s", line);}
        i++;
    }
    //branch
    char merge_branch_name[MAX_COMMIT_MESSAGE_LENGTH];
    printf("please enter the name of merged branch:");
    scanf("%s",merge_branch_name);
    fprintf(file,"branch: %s\n",merge_branch_name);
    //message
    char merge_message[MAX_COMMIT_MESSAGE_LENGTH];
    printf("please enter the message of merged branch:");
    scanf("%s",merge_message);
    fprintf(file,"message: %s\n",merge_message);
    //time
    time_t currentTime;
    time(&currentTime);
    // Convert it to a string
    char* timeString = ctime(&currentTime);
    fprintf(file,"time: %s",timeString);
    //files
    fprintf(file,"files:\n");
    FILE *file3=fopen(file_path_read_1,"r");
    FILE *file4=fopen(file_path_read_2,"r");
    char line1[MAX_LINE_LENGTH];
    char line2[MAX_LINE_LENGTH];
    i=0;
    while (fgets(line1, sizeof(line1), file3) != NULL) {
        //printf("%s",line1);
        if(i>4){fprintf(file,"%s",line1);}
        i++;
    }
    fclose(file3);
    
    int k=0;
    int flag_tekrari=0;
    while (fgets(line2, sizeof(line2), file4) != NULL) {
        //printf("%d",k);
        //printf("#%s",line2);
        if(k>4){
            //printf("^");
            i=0;
            file3=fopen(file_path_read_1,"r");
            flag_tekrari=0;
            while (fgets(line1, sizeof(line1), file3) != NULL) {
                //printf("*");
                //printf("@%s",line1);
                
                int j=0;
                char name1[100];
                while(1){
                        if(line1[j]==' '){break;}
                        name1[j]=line1[j];
                        j++;
                    }

                    name1[j]='\0';
                    //printf("*%s*\n",line2);
                    //printf("%s\n",line1);
                if(i>4){
                    char name2[100];
                    j=0;
                    while(1){
                        if(line2[j]==' '){break;}
                        name2[j]=line2[j];
                        j++;
                    }
                    name2[j]='\0';
                    printf("^%s %s^\n",name1,name2);
                    printf("%d",strcmp(name1,name2));
                    if(strcmp(name1,name2)==0){flag_tekrari=1;printf("*");}
                }
            i++;
            }
            fclose(file3);
            printf("%d",flag_tekrari);
            if(flag_tekrari==0){fprintf(file,"%s",line2);}
        }

        k++;
    }


    FILE* file5=fopen(".neogit/branches", "a");
    fprintf(file5, "merge\n");

    fclose(file5);
    fclose(file);
    fclose(file2);
    //fclose(file3);
    fclose(file4);
    printf("branches merged successfully\n");
}
int run_merge(int argc,char * const argv[]){
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir,".neogit/commits");
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;
    char last_file[MAX_FILENAME_LENGTH];
    last_file[0]='0';
    last_file[1]='\0';
    int max =0;
    while((entry = readdir(dir)) != NULL) {
            char file_path[MAX_FILENAME_LENGTH];
            strcpy(file_path,".neogit/commits/");
            strcat(file_path,entry->d_name);
            FILE * file=fopen(file_path,"r");
            
            char line[MAX_LINE_LENGTH];
            line[0]='\0';
            char ezafi[50];
            char branch[30];
            int i=0;
            char name_file[1000];
            if(file==NULL){printf("error");return 1;}
            while (fgets(line, sizeof(line), file) != NULL) {
                //printf("%s",line);
                if(i==0){fscanf(file,"%s %s",ezafi,branch);
                if(strcmp(argv[3],branch)==0){strcpy(last_file,entry->d_name);int temp=atoi(last_file);if(temp>max){max=temp;}}}
                i++;
            }
            fclose(file);
    //}
    }
    char tmp[30];
    sprintf(tmp,"%d",max);
    char file_path_read_1[MAX_FILENAME_LENGTH];
    strcpy(file_path_read_1,".neogit/commits/");
    strcat(file_path_read_1,tmp);


    strcpy(filepath_dir,".neogit/commits");
    dir = opendir(filepath_dir);
    //struct dirent *entry;
    if (dir == NULL) return 1;
    last_file[0]='0';
    last_file[1]='\0';
    max =0;
    while((entry = readdir(dir)) != NULL) {
            char file_path[MAX_FILENAME_LENGTH];
            strcpy(file_path,".neogit/commits/");
            strcat(file_path,entry->d_name);
            FILE * file=fopen(file_path,"r");
            char line[MAX_LINE_LENGTH];
            line[0]='\0';
            int i=0;
            char ezafi[50];
            if(file==NULL){printf("error");return 1;}
            while (fgets(line, sizeof(line), file) != NULL) {
                //printf("%s",line);
                if(i==0){fscanf(file,"%s %s",ezafi,branch);
                if(strcmp(argv[4],branch)==0){strcpy(last_file,entry->d_name);int temp=atoi(last_file);if(temp>max){max=temp;}}}
                i++;
            }
            fclose(file);

    }
    sprintf(tmp,"%d",max);
    char file_path_read_2[MAX_FILENAME_LENGTH];
    strcpy(file_path_read_2,".neogit/commits/");
    strcat(file_path_read_2,tmp);
    printf("%s %s\n",file_path_read_1,file_path_read_2);
    FILE *file_read_1=fopen(file_path_read_1,"r");
    FILE *file_read_2=fopen(file_path_read_2,"r");
    char line[MAX_LINE_LENGTH];
    char files_1[500][600];
    char files_2[500][600];

    line[0]='\0';
    int flag_6=0;  
    int i1=0; 
    while (fgets(line, sizeof(line), file_read_1) != NULL) {
        if(flag_6==1){strcpy(files_1[i1],line);i1++;}
        if(strstr(line,"file")){flag_6=1;}  
        }
    line[0]='\0';
    flag_6=0;  
    int i2=0; 
    while (fgets(line, sizeof(line), file_read_2) != NULL) {
        if(flag_6==1){strcpy(files_2[i2],line);i2++;}
        if(strstr(line,"file")){flag_6=1;}  
    }
    char name_1[500][500];
    char num_1[500][500];
    char name_2[500][500];
    char num_2[500][500];
    for(int k=0;k<i1;k++){
        sscanf(files_1[k], "%s %s\n", name_1[k], num_1[k]);
        //printf("%s *%s %s*\n",files_1[k],name_1[k],num_1[k]);
    }
    for(int k=0;k<i1;k++){
        sscanf(files_2[k], "%s %s\n", name_2[k], num_2[k]);
    }
    //allow to merge or not
    int d;
    int flag_allow=1;
    for(int k=0;k<i1;k++){
        int flag_same=0;
        for(int x=0;x<i2;x++){
            if(strcmp(name_1[k],name_2[x])==0){if(strcmp(num_1[k],num_2[x])!=0){d=run_diff_merge(name_1[k],name_2[x],num_1[k],num_2[x]);if(d==1){flag_allow=0;}}}
    }
    //run_diff_marge(files_1[k],files_2[k]);
    }
    FILE *file3=fopen(".neogit/merge","r");
    char v;
    fscanf(file3,"%c",&v);
    if(flag_allow==0){printf("you are not allowed to merge\n");}
    else {merge(file_path_read_1,file_path_read_2);}
    return 0;
}
int run_revert(int argc,char * const argv[]){
    int flag_message=0;
    //tmp==commit-id
    char tmp[30];
    if(strstr(argv[2],"m")){flag_message=1;strcpy(tmp,argv[4]);}
    else{strcpy(tmp,argv[2]);}
    char file_path_read[MAX_FILENAME_LENGTH];
    strcpy(file_path_read,".neogit/commits/");
    strcat(file_path_read,tmp);

    char file_path_write[MAX_FILENAME_LENGTH];
    strcpy(file_path_write,".neogit/commits/");
    int last_exist_file=inc_last_commit_ID();
    //last_exist_file++;
    printf("%d",last_exist_file);
    int check_num_sahih=atoi(tmp);
    char tmp_2[30];
    sprintf(tmp_2,"%d",last_exist_file);
    strcat(file_path_write,tmp_2);
    printf("%s",file_path_write);
    FILE*file=fopen(file_path_read,"r");
    FILE*file1=fopen(file_path_write,"w");
    char line[MAX_LINE_LENGTH];
    int i=0;
    //printf("$");
    while (fgets(line, sizeof(line), file) != NULL) {

            if(i>3){
                char name[100];char num[20];
                int j=0;
                while(1){
                    if(line[j]==' '){break;}
                    name[j]=line[j];
                    j++;
                }
                name[j]='\0';
                j++;
                while(1){
                    if(line[j]=='\0'){break;}
                    name[j]=line[j];
                    j++;
                }
            //ino ezafe kardam vase commit
                if(argc==5){strcpy(argv[3],argv[4]);}
                else{int last_id=find_file_last_commit(name);
                    char num[30];
                    sprintf(num,"%d",last_id);
                    strcpy(argv[3],num);
                }
                run_checkout_commit_id(argc,argv);
            }
            if(i==2){
                if(strstr(argv[2],"m")){fprintf(file1,"message: %s\n",argv[3]);}
                else{fprintf(file1,"%s",line);}
                i++;
                continue;
            }
            i++;
            fprintf(file1,"%s",line);
            }
            //ino bara bargashtn ezafe kardam
            //
        FILE *file2=fopen(".neogit/allow","w");
        fprintf(file2,"y");
        fclose(file2);
    return 0;
}
int run_revert_n(int argc,char * const argv[]){
    //printf("*");
    char tmp[30];
    char file_path_read[MAX_FILENAME_LENGTH];
    strcpy(file_path_read,".neogit/commits/");
    strcpy(tmp,argv[3]);
    strcat(file_path_read,tmp);
    FILE*file=fopen(file_path_read,"r");

    char line[MAX_LINE_LENGTH];
    int i=0;
    while (fgets(line, sizeof(line), file) != NULL) {

            if(i>3){
                char name[100];char num[20];
                int j=0;
                while(1){
                    if(line[j]==' '){break;}
                    name[j]=line[j];
                    j++;
                }
                name[j]='\0';
                j++;
                while(1){
                    if(line[j]=='\0'){break;}
                    name[j]=line[j];
                    j++;
                }
            //ino ezafe kardam vase commit
                if(argc==4){}
                else{int last_id=find_file_last_commit(name);
                    char num[30];
                    sprintf(num,"%d",last_id);
                    strcpy(argv[3],num);
                }
                run_checkout_commit_id(argc,argv);
            }
            i++;
            }
            //ino bara bargashtn ezafe kardam
            //
        FILE *file2=fopen(".neogit/allow","w");
        fprintf(file2,"y");
        fclose(file2);
    return 0;
}
int run_revert_head(int argc,char * const argv[]){
    int flag_message=0;
    //tmp==commit-id
    char tmp[30];
    if(strstr(argv[2],"m")){flag_message=1;strcpy(tmp,argv[5]);}
    else{strcpy(tmp,argv[4]);}
    int number=atoi(tmp);
    int last_num=inc_last_commit_ID_bedoon_taghirat();
    number=last_num-number;
    sprintf(tmp,"%d",number);
    char file_path_read[MAX_FILENAME_LENGTH];
    strcpy(file_path_read,".neogit/commits/");
    strcat(file_path_read,tmp);

    char file_path_write[MAX_FILENAME_LENGTH];
    strcpy(file_path_write,".neogit/commits/");
    //tafavot
    int last_exist_file=inc_last_commit_ID();
    int check_num_sahih=atoi(tmp);;
    char tmp_2[30];
    sprintf(tmp_2,"%d",last_exist_file);
    strcat(file_path_write,tmp_2);
    //printf("%s",file_path_write);
    //printf("%s",file_path_read);
    
    
    FILE*file=fopen(file_path_read,"r");
    FILE*file1=fopen(file_path_write,"w");
    char line[MAX_LINE_LENGTH];
    int i=0;
    //printf("$");
    while (fgets(line, sizeof(line), file) != NULL) {

            if(i>3){
                char name[100];char num[20];
                int j=0;
                while(1){
                    if(line[j]==' '){break;}
                    name[j]=line[j];
                    j++;
                }
                name[j]='\0';
                j++;
                while(1){
                    if(line[j]=='\0'){break;}
                    name[j]=line[j];
                    j++;
                }
            //ino ezafe kardam vase commit
                if(argc==6){strcpy(argv[3],argv[5]);}
                else{int last_id=find_file_last_commit(name);
                    char num[30];
                    sprintf(num,"%d",last_id);
                    strcpy(argv[3],num);
                }
                run_checkout_commit_id(argc,argv);
            }
            if(i==2){
                if(strstr(argv[2],"m")){fprintf(file1,"message: %s\n",argv[3]);}
                else{fprintf(file1,"%s",line);}
                i++;
                continue;
            }
            i++;
            fprintf(file1,"%s",line);
            }
            //ino bara bargashtn ezafe kardam
            //
        FILE *file2=fopen(".neogit/allow","w");
        fprintf(file2,"y");
        fclose(file2);
    return 0;
}
int run_diff_commit(int argc,char * const argv[]){
    char file_path_read_1[MAX_FILENAME_LENGTH];
    strcpy(file_path_read_1,".neogit/commits/");
    strcat(file_path_read_1,argv[3]);

    char file_path_read_2[MAX_FILENAME_LENGTH];
    strcpy(file_path_read_2,".neogit/commits/");
    strcat(file_path_read_2,argv[4]);
    //printf("%s %s\n",file_path_read_1,file_path_read_2);
    FILE *file_read_1=fopen(file_path_read_1,"r");
    FILE *file_read_2=fopen(file_path_read_2,"r");
    char line[MAX_LINE_LENGTH];
    char files_1[500][600];
    char files_2[500][600];

    line[0]='\0';
    int flag_6=0;  
    int i1=0; 
    while (fgets(line, sizeof(line), file_read_1) != NULL) {
        if(flag_6==1){strcpy(files_1[i1],line);i1++;}
        if(strstr(line,"file")){flag_6=1;}  
        }
    line[0]='\0';
    flag_6=0;  
    int i2=0; 
    while (fgets(line, sizeof(line), file_read_2) != NULL) {
        if(flag_6==1){strcpy(files_2[i2],line);i2++;}
        if(strstr(line,"file")){flag_6=1;}  
    }
    char name_1[500][500];
    char num_1[500][500];
    char name_2[500][500];
    char num_2[500][500];
    for(int k=0;k<i1;k++){
        sscanf(files_1[k], "%s %s\n", name_1[k], num_1[k]);
        //printf("%s *%s %s*\n",files_1[k],name_1[k],num_1[k]);
    }
    for(int k=0;k<i1;k++){
        sscanf(files_2[k], "%s %s\n", name_2[k], num_2[k]);
    }
    for(int k=0;k<i1;k++){
        int flag_same=0;
        for(int x=0;x<i2;x++){
            //printf("%s %s\n",name_1[k],name_2[x]);
            if(strcmp(name_1[k],name_2[x])==0){if(strcmp(num_1[k],num_2[x])!=0){run_diff_merge(name_1[k],name_2[x],num_1[k],num_2[x]);}}
    }
    //run_diff_marge(files_1[k],files_2[k]);
    }
    return 0;}
//int run_add_file_situation(){}
int run_hook_list(){
    printf("hooks list:\n");
    printf("todo-check\n");
    printf("eof-blank-space\n");
    printf("format-check\n");
    printf("balance-braces\n");
    printf("indentation-check\n");
    printf("static-error-check\n");
    printf("file-size-check\n");
    printf("character-limit\n");
    printf("time-limit\n");
    return 0;
}
int run_hook_applied(){
    FILE*file=fopen(".neogit/hook","r");
    if(file==NULL){printf("no hooks applied\n");return 0;}
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s",line);
    }
    //printf("hook applied successfully\n");
    return 0;
}
int run_hook_add(int argc,char * const argv[]){
    FILE*file=fopen(".neogit/hook","a");
    fprintf(file,"%s\n",argv[4]);
    printf("hook added successfully\n");
    fclose(file);
    return 0;
}
int run_hook_remove(int argc,char * const argv[]){
    FILE*file=fopen(".neogit/hook","r");
    FILE*file2=fopen(".neogit/tmp","w");
    if(file==NULL){printf("no hooks applied\n");return 0;}
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);
        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(argv[4], line) == 0){}
        else{fprintf(file2,"%s\n",line);}
    }
    printf("hook added successfully\n");
    fclose(file);
    fclose(file2);
    remove(".neogit/hook");
    rename(".neogit/tmp", ".neogit/hook");
    return 0;
}
int check_size(char * filename){
    //return 0;
    FILE*file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Unable to open the file.\n");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fclose(file);

    double sizeInMB = (double)filesize / (1024 * 1024);
    
    if (sizeInMB > 5) {
        printf("File size exceeds 5 MB.\n");
        return 1;
    } else {
        printf("File size is less than or equal to 5 MB.\n"); 
    }
    return 0;
}
int check_space(char * filename){
    //printf("%s",filename);
    if (strstr(filename, ".cpp") || strstr(filename, ".c") || strstr(filename, ".txt")) {
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            printf("Unable to open the file.\n");
            return 0;
        }
    
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
    
        if (size == -1) {
            printf("Error in determining file size.\n");
            fclose(file);
            return 0;
        }
    
        bool noWhitespace = true;
        char ch;
        while (size > 0) {
            fseek(file, size-1, SEEK_SET);
            //size--;
            ch = fgetc(file);
            //printf("%c",ch);
            if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
                //printf("%c",ch);
                noWhitespace = false;
                break;
            }
            size--;
        }
    
        fclose(file);

        if (noWhitespace) {
            return 0;
        } else {
            printf("this file have null spaces\n");
            return 1;
        }
    } else {
        return 0;
    }
}
int check_hooks(char *filepath){
    char line[MAX_LINE_LENGTH];
    FILE*file=fopen(".neogit/hook","r");
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);
        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if(line[5]=='s' && line[7]=='z'){
            int d=check_size(filepath);if(d==1){return 1;}
        }
        if(line[0]=='e' && line[2]=='f'){
            int d=check_space(filepath);if(d==1){return 1;}
        }
    }
    return 0;
}
int check_hook_size(char * filename){
    //return 0;
    FILE*file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Unable to open the file.\n");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long int filesize = ftell(file);
    fclose(file);

    double sizeInMB = (double)filesize / (1024 * 1024);
    //printf("%lf",sizeInMB);
    if (sizeInMB > 5) {
        printf(RED_COLOR"file-size-check........................................FAILED\n"RESET_COLOR);
        return 1;
    } else {
        printf(BLUE_COLOR"file-size-check........................................PASSED\n"RESET_COLOR);
        
    }
    return 0;
}
int check_hook_space(char * filename){
    //printf("%s",filename);
    if (strstr(filename, ".cpp") || strstr(filename, ".c") || strstr(filename, ".txt")) {
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            printf("Unable to open the file.\n");
            return 1;
        }
    
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
    
        if (size == -1) {
            printf("Error in determining file size.\n");
            fclose(file);
            return 1;
        }
    
        bool noWhitespace = true;
        char ch;
        while (size > 0) {
            fseek(file, size-1, SEEK_SET);
            //size--;
            ch = fgetc(file);
            //printf("%c",ch);
            if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
                //printf("%c",ch);
                noWhitespace = false;
                break;
            }
            size--;
        }
    
        fclose(file);

        if (noWhitespace) {
            printf(BLUE_COLOR"eof-blank-space.........................................PASSED\n"RESET_COLOR);
            //printf("No whitespace characters at the end of the file.\n");
        } else {
            printf(RED_COLOR"eof-blank-space.........................................FAILED\n"RESET_COLOR);
            //printf("Whitespace characters found at the end of the file.\n");
        }
    } else {
        printf(YELLOW_COLOR"eof-blank-space.........................................SKIPPED\n"RESET_COLOR);
    }
}
int run_precommit(){
    char line[MAX_LINE_LENGTH];
    FILE*file=fopen(".neogit/staging","r");
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);
        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        printf("%s\n",line);
        check_hook_size(line);
        check_hook_space(line);
    }
    return 0;
}
int main(int argc, char *argv[]) {
    char username[200];
    char email[200];
    if (argc < 2) {
        fprintf(stdout, "please enter a valid command");
        return 1;
    }
    
    print_command(argc, argv);
    //run_add_file_situation();
    run_check_alias(argc,argv);
    if (strcmp(argv[1], "init") == 0) {
        return run_init(argc, argv);
    } else if (strcmp(argv[1], "add") == 0) {
        if(argc<3){printf("invalid command");return 0;}
        if(strcmp(argv[2],"-f")==0){
            for(int i=3;i<argc;i++){
             run_add_chand(argc, argv[i]);}}
             else if(strcmp(argv[2],"-n")==0){run_depth(argc,argv);}
             else if(strcmp(argv[2],"-redo")==0){run_redo(argc,argv);}
            else if(strstr(argv[2],"*")){
                if(argc<2){printf("invalid command\n");}
                //wild card
                else{run_wildcard(argc,argv[2]);}
            }
        else{run_add_chand(argc, argv[2]);}
    } else if (strcmp(argv[1], "reset") == 0) {
        if(strcmp(argv[2],"-undo")==0){run_reset_undo(argc, argv);}
        else if(strcmp(argv[2],"-f")==0){for(int i=3;i<argc;i++){
             run_reset(argc, argv[i]);}}
             else if(strstr(argv[2],"*")){
                //wild card
                if(argc<2){printf("invalid command\n");}
                else{run_wildcard_reset(argc,argv[2]);}}
        else{return run_reset(argc, argv[2]);}
    } else if (strcmp(argv[1], "status") == 0) {
        if(argc<2){printf("invalid command");return 0;}
        return run_status(argc, argv);
    } else if (strcmp(argv[1], "commit") == 0) {
        FILE*file=fopen(".neogit/allow","r");if(file!=NULL){char d[50];fscanf(file,"%s",d);if(d[0]=='n'){printf("you are not allowed to commit\n");return 0;}}
        if(strstr(argv[2],"m")){return run_commit(argc, argv);}
        else if(strstr(argv[2],"s")){;return run_commit(argc, argv);}
        else{printf("invalid command");}
    } else if (strcmp(argv[1], "config") == 0) {
        if(strstr(argv[2],"alias")){return run_alias(argc,argv);}
        else if(strstr(argv[2],"global")){
            if(argc<4){printf("invalid command");return 0;}
            if(strcmp(argv[3],"username")==0){printf("global-username sets\n");FILE *file5 = fopen(".neogit/global", "a");
            fprintf(file5,"%s",argv[4]);fprintf(file5,"\n");fclose(file5);strcpy(username1,argv[3]);}
            if(strcmp(argv[3],"email")==0){printf("global-email sets\n");FILE *file5 = fopen(".neogit/global", "a");
            fprintf(file5,"%s",argv[4]);fprintf(file5,"\n");fclose(file5);strcpy(username1,argv[3]);create_configs(username1, email1);}}
        else if(strcmp(argv[2],"username")==0){printf("username sets\n");FILE* user_e=fopen(".neogit/user_e","w");fprintf(user_e,"%s",argv[3]);}
        else if(strcmp(argv[2],"email")==0){printf("email sets\n");FILE* user_e=fopen(".neogit/user_e","r");fscanf(user_e,"%s",username);create_configs(username,argv[3]);remove(".neogit/user_e"); }
    } else if (strcmp(argv[1], "set") == 0) {
        if(strstr(argv[2],"m")==0){printf("invalid command\n");return 0;}
        if(strstr(argv[4],"s")==0){printf("invalid command\n");return 0;}
        return run_commit_set(argc, argv);
    } else if (strcmp(argv[1], "replace") == 0) {
        if(strstr(argv[2],"m")==0){printf("invalid command\n");return 0;}
        if(strstr(argv[4],"s")==0){printf("invalid command\n");return 0;}
        return run_commit_replace(argc, argv);
    } else if (strcmp(argv[1], "remove") == 0) {
        if(strstr(argv[2],"s")==0){printf("invalid command\n");return 0;}
        return run_commit_remove(argc, argv);
    } else if (strcmp(argv[1], "log") == 0) {
        if(argc>2){
            if(strstr(argv[2],"branch")){return run_log_branch(argc,argv);}
            else if(strstr(argv[2],"author")){return run_log_author(argc,argv);}
            else if(strstr(argv[2],"since")){return run_log_since(argc,argv);}
            else if(strstr(argv[2],"before")){return run_log_before(argc,argv);}
            else if(strstr(argv[2],"search")){return run_log_search(argc,argv);}
            else if(strstr(argv[2],"n")){return run_log_chand(argv);}
            }
        else{return run_log();}
    } else if (strcmp(argv[1], "branch") == 0) {
        if(argv[2]!=NULL){return run_branch(argc, argv);}
        else{run_print_all_branches();}
        //?????
        
    } else if (strcmp(argv[1], "tag") == 0) {
        //printf("%d",argc);
        if(argc==2){run_print_all_tags(argc, argv);}
        else if(strcmp(argv[2],"show")==0){run_tags_show(argc, argv);}
        else{
        run_tags(argc, argv);}
    }else if (strcmp(argv[1], "diff") == 0) {
        if(strstr(argv[2],"f")){run_diff(argc,argv);}
        else if(strstr(argv[2],"c")){run_diff_commit(argc,argv);}
        else{printf("invalid command\n");}
    } else if (strcmp(argv[1], "revert") == 0) {
        if(strstr(argv[2],"n")){run_revert_n(argc,argv);}
        else if(argc<6){run_revert(argc,argv);}
        else if(argc==6){run_revert_head(argc,argv);}

    }  else if (strcmp(argv[1], "merge") == 0) {
        if(strstr(argv[2],"b")==0){printf("invalid command");return 0;}
        if(argc<5){printf("invalid command");return 0;}
        run_merge(argc,argv);
    }
    else if (strcmp(argv[1], "checkout") == 0) {
        if(strstr(argv[2],"commit")){run_checkout_commit_id(argc,argv);}
        else if(strcmp(argv[2],"branch")==0){run_checkout_branch(argc,argv);}
        else if(strcmp(argv[2],"HEAD")==0){FILE*file=fopen(".neogit/allow","w");fprintf(file,"y");}
        else{printf("invalid command");}
    } else if (strcmp(argv[1], "grep") == 0) {
        printf("%d",argc);
        run_grep(argc,argv);
    } else if (strcmp(argv[1], "pre-commit") == 0) {
        if(argc>2){
        if(strcmp(argv[2],"hooks")==0 && strcmp(argv[3],"list")==0){run_hook_list();}
        else if(strcmp(argv[2],"applied")==0 && strcmp(argv[3],"hooks")==0){run_hook_applied();}
        else if(strcmp(argv[2],"add")==0 && strcmp(argv[3],"hook")==0){run_hook_add(argc,argv);}
        else if(strcmp(argv[2],"remove")==0 && strcmp(argv[3],"hook")==0){run_hook_remove(argc,argv);}
        else{printf("invalid command\n");}}
        else{run_precommit();}
    }
    else{fprintf(stdout,"invalid command\n");}
    
    return 0;
}
