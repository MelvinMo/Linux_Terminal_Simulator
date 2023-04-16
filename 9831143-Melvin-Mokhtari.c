#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// user structure
struct Users
{
    char *fullname;
    char *username;
    char *password;
    int password_strength;
    int permission;
    int incorrect_commands;
    int expiration_date; // Example: 2020127
};

// logged-in user
struct Users user;
int isLoggedIn = 0;

// current date
int date;

struct Command
{
    char *pre;
    char *command;
    char *post;
    char *switch_key;
    char *switch_value;
    char *fullcommand;
};

// current command
struct Command command;

struct Files
{
    char *filename;
    char *user;
    char *group;
    int lastAccess;
};

int hasExpired()
{
    if (user.expiration_date - date < 1)
    {
        return 1;
    }

    return 0;
}

// delimit by pipe (|)
struct Users extract_user_info(char *str)
{
    const char s[] = "|";
    char *token;
    struct Users this_user;

    /* get the first token */
    token = strtok(str, s);

    /* walk through other tokens */
    int i = 0;
    while (token != NULL)
    {
        switch (i)
        {
        case 0:
            this_user.fullname = token;
            break;
        case 1:
            this_user.username = token;
            break;
        case 2:
            this_user.password = token;
            break;
        case 3:
            this_user.password_strength = atoi(token);
            break;
        case 4:
            this_user.incorrect_commands = atoi(token);
            break;
        case 5:
            this_user.permission = atoi(token);
            break;
        case 6:
            this_user.expiration_date = atoi(token);
            break;
        default:
            break;
        }
        token = strtok(NULL, s);
        i++;
    }

    return this_user;
}

size_t myGetline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}

// login
void login(char *username, char *password)
{
    char *line = NULL;
    size_t size;
    int userFound = 0;

    FILE *fp = fopen("root/users.txt", "r");

    if (fp == NULL)
    {
        return;
    }

    while (myGetline(&line, &size, fp) != -1)
    {
        struct Users this_user = extract_user_info(line);

        if (strcmp(this_user.username, username) == 0)
        {
            userFound = 1;
            if (
                strcmp(this_user.password, password) == 0 ||
                (isLoggedIn == 1 && user.permission == 1))
            {
                user = this_user;

                if (hasExpired() == 1)
                {
                    isLoggedIn = 0;
                    printf("-> This account has been disabled.\n");
                }
                else
                {
                    isLoggedIn = 1;
                }

                return;
            }
        }
    }

    if (userFound == 0 || isLoggedIn == 0)
    {
        printf("-> username & password don't matches.\n");
        printf("Please retry.\n");
    }

    return;
}

// clear user session
int logout()
{
    struct Users user;
}

int useradd()
{
}

int passwordStrength(char password[])
{
    int i, score = 0;
    char c;

    for (i = 0; i < strlen(password); i++)
    {
        c = password[i];

        if (c >= 'a' && c <= 'z')
        {
            score += 2;
        }

        if (c >= 'A' && c <= 'Z')
        {
            score += 4;
        }

        // Numbers
        if (c >= 48 && c <= 57)
        {
            score += 3;
        }

        // Punctuations
        if (c >= 33 && c <= 47)
        {
            score += 8;
        }

        // Space
        if (c == 32)
        {
            score += 1;
        }
    }

    if (score > 100)
    {
        return 100;
    }

    return score;
}

void commandParser(char *cmnd)
{
    const char s[] = " ";
    char *token;
    int i, count = 0;

    for (i = 0; i < strlen(cmnd); i++)
    {
        if (cmnd[i] == ' ')
            count++;
    }

    /* get the first token */
    token = strtok(cmnd, s);

    /* walk through other tokens */
    i = 0;
    while (token != NULL)
    {
        if (count == 0)
        {
            command.command = token;
        }

        if (count == 1)
        {
            if (i == 0)
            {
                command.command = token;
            }

            if (i == 1)
            {
                command.post = token;
            }
        }

        if (count == 2)
        {
            if (i == 0)
            {
                command.pre = token;
            }

            if (i == 1)
            {
                if (strcmp(token, ">>") == 0)
                {
                    command.command = "append";
                }
                else if (strcmp(token, ">") == 0)
                {
                    command.command = "redirect";
                }
                else
                {
                    command.command = token;
                }
            }

            if (i == 2)
            {
                command.post = token;
            }
        }

        token = strtok(NULL, s);
        i++;
    }

    return;
}

void su()
{
    int suLoggedIn = 0;
    char password[50];

    if (command.command != NULL &&
        command.post != NULL)
    {
        if (user.permission == 1)
        {
            login(command.post, "");
        }
        else
        {
            while (suLoggedIn == 0)
            {
                printf("Please enter password: ");
                scanf("%s", password);
                login(command.post, password);
                suLoggedIn = 1;
            }
        }
    }
}

void passwd()
{
    char password[50];

    if (command.command != NULL)
    {
        // change other people password
        if (command.post != NULL)
        {
            if (user.permission == 1)
            {
                while (1)
                {
                    printf("Please enter a new strong password: ");
                    fgets(password, 49, stdin);

                    if (passwordStrength(password) >= 33)
                    {
                        printf("Good password!\n");
                        // TODO: get user by name & update password
                        // TODO: update on file
                        break;

                    }
                    else
                    {
                        printf("Weak password!\n");
                    }
                }
            }
        }
        else
        {
            while (1)
            {
                printf("Please enter a new strong password: ");
                fgets(password, 49, stdin);

                if (passwordStrength(password) >= 33)
                {
                    printf("Good password!\n");
                    user.password = password;
                    break;
                    // TODO: update on file
                }
                else
                {
                    printf("Weak password!\n");
                }
            }
        }
    }

    // TODO: -l switch
}

void createuser()
{
    struct Users new_user;
    char fullname[50], username[50], password[50];
    int permission, expiration;

    printf("Please enter the new user's fullname: ");
    fgets(fullname, 49, stdin);

    printf("Please enter the new user's username: ");
    fgets(username, 49, stdin);

    // check password strength
    while (1)
    {
        printf("Please enter a strong password: ");
        fgets(password, 49, stdin);

        if (passwordStrength(password) >= 33)
        {
            printf("Good password!\n");
            break;
        }
        else
        {
            printf("Weak password!\n");
        }
    }

    printf("Please enter permission ( 0 : normal / 1 : admin ): ");
    scanf("%d", &permission);

    printf("Please enter expiration date (like: 2020127): ");
    scanf("%d", &expiration);

    new_user.fullname = fullname;
    new_user.username = username;
    new_user.password = password;
    new_user.password_strength = passwordStrength(password);
    new_user.permission = permission;
    new_user.incorrect_commands = 0;
    new_user.expiration_date = expiration; // Example: 2020127

    // TODO: save to file
}

void chgr()
{
    if (user.permission != 1)
    {
        printf("You are not admin!\n");
        return;
    }

    if (command.post != NULL)
    {
        // TODO find user by name
        struct Users this_user;

        if (this_user.permission == 0 &&
            passwordStrength(this_user.password) >= 75 &&
            this_user.incorrect_commands <= 10)
        {
            this_user.permission = 1;
            // TODO: update to file
        }
    }
}

void setPermission(char filename)
{
    /*
    TODO:
        metadata.txt
            filename,user,group
    */
}

void hasPermission(char filename)
{
    /*
    TODO:
        check user related to filename in metadata.txt
    */
}

//
void command_translate(const char *needle, const char *replacement)
{
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = command.fullcommand;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(command.fullcommand, buffer);
}

// This part can be compiled easily on Linux without even mentioning! Sophisticated Windows!
char *unixToWindows()
{
  if (strstr(command.fullcommand, "cd") != NULL)
  {
      command_translate("cd", "cd");
  }

  else if (strstr(command.fullcommand, "pwd") != NULL)
  {
      command_translate("pwd", "chdir");
  }

  else if (strstr(command.fullcommand, "mkdir") != NULL)
  {
      command_translate("mkdir", "mkdir");
  }

  else if (strstr(command.fullcommand, "redirect") != NULL)
  {
      command_translate(">", "type >");
  }

  else if (strstr(command.fullcommand, "append") != NULL)
  {
      command_translate(">>", "type >>");
  }

  else if (strstr(command.fullcommand, "cat") != NULL)
  {
      command_translate("cat", "type");
  }

  else if (strstr(command.fullcommand, "rm") != NULL)
  {
    command_translate("rm", "remove");
  }

  else if (strstr(command.fullcommand, "cp") != NULL)
  {
      command_translate("cp", "copy");
  }

  else if (strstr(command.fullcommand, "mv") != NULL)
  {
      command_translate("mv", "move");
  }

  else if (strstr(command.fullcommand, "ls") != NULL)
  {
      command_translate("ls", "dir");
  }

  return command.fullcommand;
}

void cd()
{
  unixToWindows();
  system(command.fullcommand);
}

void pwd()
{
  unixToWindows();
  system(command.fullcommand);
}

void mkdir()
{
  unixToWindows();
  system(command.fullcommand);
}

void redirect()
{
  unixToWindows();
  system(command.fullcommand);
}

void append()
{
  unixToWindows();
  system(command.fullcommand);
}

void cat()
{
  unixToWindows();
  system(command.fullcommand);
}

void rm()
{
  unixToWindows();
  system(command.fullcommand);
}

void cp()
{
  unixToWindows();
  system(command.fullcommand);
}

void mv()
{
  unixToWindows();
  system(command.fullcommand);
}

void ls()
{
  unixToWindows();
  system(command.fullcommand);
}

void exif()
{
    // TODO: shows metadata.txt
}
//Reading text from file in editor
void textReader(char filename[])
{
    int c;
    FILE *file;
    file = fopen(filename, "r");
    if (file)
    {
        while ((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
}

void myEditor()
{
    if (command.post != NULL)
    {
        textReader(command.post);
    }

    if (command.post == NULL)
    {
        // to be continued...
    }
}

int main()
{
    printf("               ******************************************************************************");
    printf("\n\nHello and welcome to this Linux terminal simulator!\nThis simple letters and digits have created a successful program by the sweat of Melvin Mokhtari's brow.\n");
    printf("Enjoy using it!\n\n               ******************************************************************************\n\n");
    // get current time
    struct tm *current_time;
    time_t s = time(NULL);
    current_time = localtime(&s);

    // memory allocation
    char cmnd[50], input[50], root[50], username[50], password[50];

    // current date
    date =
        (current_time->tm_year + 1900) * 1000 +
        (current_time->tm_mon + 1) * 100 +
        current_time->tm_mday;

   while (isLoggedIn == 0)
    {
        printf("Please enter your username: ");
        scanf("%s", username);

        printf("Please enter your password: ");
        scanf("%s", password);

        login(username, password);
    }

    // change user directory
    //printf("Please enter root directory name: ");
    //scanf("%s", &root);

    printf("If you do not now what to do type 'help' or for more information type 'helpcommand' else enter 'exit' to quit:\n");

    while (1)
    {
        printf("-> %s: ", user.username);
        fgets(input, 49, stdin);

        // remove \n
        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            logout();
            return 0;
        }
        if (strcmp(input, "help") == 0)
        {
            printf("Type these commands in order to :\n");
            printf("       su Command:To change user.\n");
            printf("   passwd Command:To change Password.\n");
            printf("creatuser Command:To add new user.\n");
            printf("     chgr Command:To upgrade access level of user.\n");
            printf("       cd Command:To change current directory.\n");
            printf("      pwd Command:To show where the user is.\n");
            printf("    mkdir Command:To add new directory.\n");
            printf("        > Command:To print a expression in the current file.\n");
            printf("       >> Command:To print a expression in the next file.\n");
            printf("      cat Command:To show the contents of a file.\n");
            printf("       rm Command:To delete a file.\n");
            printf("       cp Command:To copy files in each other.\n");
            printf("       mv Command:To move files in each other.\n");
            printf("       ls Command:To show the contents of current directory.\n");
            printf("     exif Command:To show the details of a file.\n");
            printf(" myeditor Command:To open the editor.\n");
            printf("       wc Command:To show the numbers of words and lines in a file.\n");
            printf("     diff Command:To compare two different file.\n");
            printf("NOW YOU CAN UNDERSTAND THIS PROGRAM BETTER!\n");
            printf("FOR MORE INFORMATION PLEASE TYPE 'helpcommand'.\n");
            printf("\n");
        }
        if (strcmp(input, "helpcommand") == 0)
        {
            printf("These are good and more detailed information about commands :\n");
            printf("       su Command:The Linux command 'su' is used to switch from one account to another.\n");
            printf("   passwd Command:passwd command in Linux is used to change the user account passwords.\n");
            printf("creatuser Command:This command 'creatuser' is used to add new account.\n");
            printf("     chgr Command:The 'chgr' command is used to change the group that has access to files and directories.\n");
            printf("       cd Command:The 'cd' command is used to change the current directory.\n");
            printf("      pwd Command:The 'pwd' command is a command line utility for printing the current working directory.\n");
            printf("    mkdir Command:The 'mkdir' command is used to make a new directory.\n");
            printf("        > Command:To print a expression in the current file.\n");
            printf("       >> Command:To print a expression in the next file.\n");
            printf("      cat Command:To show the contents of a file.\n");
            printf("       rm Command:'rm' command is used to remove objects such as files or directories.\n");
            printf("       cp Command:The 'cp' command is a command-line utility for copying files and directories.\n");
            printf("       mv Command:The 'mv' command is a command-line utility that moves files from one place to another.\n");
            printf("       ls Command:The 'ls' command is a command-line utility for listing the contents of a directory.\n");
            printf("     exif Command:To show the details of a file.\n");
            printf(" myeditor Command:To open the editor.\n");
            printf("       wc Command:To show the numbers of words and lines in a file.\n");
            printf("     diff Command:To compare two different file.\n");
            printf("NOW DEFINATLY YOU CAN UNDERSTAND THIS PROGRAM!\n");
            printf("\n");
        }

        commandParser(input);

        // commands
        if (strcmp(input, "su") >= 0)
        {
            su();
        }

        else if (strstr(input, "passwd") != NULL)
        {
            passwd();
        }

        else if (strstr(input, "createuser") != NULL)
        {
            createuser();
        }

        else if (strstr(input, "chgr")!= NULL)
        {
            chgr();
        }

        else if (strstr(input, "cd") != NULL)
        {
            cd();
        }

        else if (strstr(input, "pwd") != NULL)
        {
            pwd();
        }

        else if (strstr(input, "mkdir") != NULL)
        {
            mkdir();
        }

        else if (strstr(input, "redirect") != NULL)
        {
            redirect();
        }

        else if (strstr(input, "append") != NULL)
        {
            append();
        }

        else if (strstr(input, "cat") != NULL)
        {
            cat();
        }

        else if (strstr(input, "rm") != NULL)
        {
            rm();
        }

        else if (strstr(input, "cp") != NULL)
        {
            cp();
        }

        else if (strstr(input, "mv") != NULL)
        {
            mv();
        }

        else if (strstr(input, "ls") != NULL)
        {
            ls();
        }

        else if (strstr(input, "exif") != NULL)
        {
            exif();
        }

        else
        {
            user.incorrect_commands++;
            // TODO: update on file
        }

        // reset command
        command.pre = NULL;
        command.command = NULL;
        command.post = NULL;
    }

    printf("-> \n");
    return 0;
}
