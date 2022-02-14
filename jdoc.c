/*
  JDOC Application
  C language command line application that reads in .java files and analyses the java code within it.
  Author: Angus Dolan
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
  Function that returns 1 if line is blank, 0 if line has data.
  blank lines are defined as lines with no data left even after stripping spaces, tabs and newline chars.
*/
int is_empty_line(char *line)
{
  char *DELIMS = " \t\n";
  char *token = strtok(line, DELIMS);

  if (token == NULL)
  {
    // Blank line found
    return 1;
  }
  else
  {
    // Line contains data outside of delimiters
    return 0;
  }
}

/*
  Function that extracts data from a string starting from a breakpoint
  • destination = string to copy final result of func to
  • data = line data to work with
  • break_string = starting point to copy data out of input string
  note: only works with no spaces in break_string
*/
void parse_from(char *destination, char *data, char *break_string)
{
  char *DELIMS = " \n";
  char *token = strtok(data, DELIMS);
  char result[101] = {0};

  int found = 0;
  while (token != NULL)
  {
    // Extract data then concatenate using sprintf into result
    if (found > 0)
      sprintf(result, "%s %s", result, token);

    // When current token is equal to break_string, set found = 1
    if (strstr(token, break_string))
      found = 1;
    // Move to next token
    token = strtok(NULL, DELIMS);
  }

  // Copy final string to destination
  strcpy(destination, result);
}

/*
  Function to cleanly get the name of a method/class directly from line data
  • destination = string to copy final result of func to
  • line_data = line data to work with
  • break_char = character which the func will end on, example with a method this would be "(".
*/
void parse_name(char *destination, char *line_data, char *break_char)
{
  strcpy(destination, line_data);

  // Tokenize
  char *DELIMS = " \n";
  char *token = strtok(destination, DELIMS);

  // Protected strings
  char *PROTECTED_STRINGS[] = {"private", "public", "class", "void", "double", "int", "String", "\0"};
  int NUM_PROTECTED_STRINGS = (int)(sizeof(PROTECTED_STRINGS) / sizeof(PROTECTED_STRINGS[0]) - 1);

  // Loop each token, break if loop reaches break_char
  while (token != NULL)
  {
    int protected = 0;

    for (int i = 0; i < NUM_PROTECTED_STRINGS; ++i)
    {
      // If current loop token is protected, break loop
      if (strcmp(token, PROTECTED_STRINGS[i]) == 0)
      {
      protected
        = 1;
        break;
      }
      // Target name is found when all protected strings have been tested against token
      if ((i == NUM_PROTECTED_STRINGS - 1) && protected == 0)
        strcpy(destination, token);
    }

    // Move to next token
    token = strtok(NULL, DELIMS);
    // If token = break_char, end the loop
    if (strcmp(token, break_char) == 0)
      break;
  }
}

/*
  Function to parse the input/output file names from argv
  • dest_input_f = string to store final input file name
  • dest_output_f = string to store final output file name
  • argc = number of cli args passed
  • argv = array of cli args passed by user in compiler
*/
void parse_cli_args(char *dest_input_f, char *dest_output_f, int argc, char *argv[])
{
  int i;
  for (i = 0; i < argc; i++)
  {
    if (strstr(argv[i], "-i"))
    {
      // Input file discovered
      strcpy(dest_input_f, argv[i + 1]);
    }
    else if (strstr(argv[i], "-o"))
    {
      // Output file discovered
      strcpy(dest_output_f, argv[i + 1]);
    }
  }
}

int main(int argc, char *argv[])
{

  // Command line arguments
  char input_f[51 * sizeof(char)];
  char output_f[51 * sizeof(char)];
  parse_cli_args(input_f, output_f, argc, argv);

  // Input file
  char dir_input_f[51 * sizeof(char)];
  sprintf(dir_input_f, "%s%s", "src/", input_f);
  FILE *input = fopen(dir_input_f, "r");

  // Output file
  FILE *output_f_pointer;
  output_f_pointer = fopen(output_f, "w");
  fprintf(output_f_pointer, "");
  fclose(output_f_pointer);
  output_f_pointer = fopen(output_f, "a");

  char line[1000];      // will store each line in turn
  char prev_line[1000]; // used to store previous line in main while loop

  // Init program counters
  int total_lines = 0;
  int blank_lines = 0;
  int num_comments = 0;

  char class_name[50];
  char file_author[50];

  int print_class_author = 0;
  int print_comment = 0; // flag that determines when to print jdoc comments

  char method_name[26 * sizeof(char)];
  char method_desc[5][101 * sizeof(char)];
  int num_method_desc = 0;
  int print_method = 0;

  // Traverse through input file
  while (fgets(line, 1000, input) != NULL)
  {
    //
    // When loop finds opening jdoc comment, begin printing
    //
    if (strstr(line, "/**") != NULL)
    {
      print_comment = 1; // set print comment flag to true
      num_comments++;    // increment num_comments when an opening jdoc comment block is found
    }

    //
    // When print_comment flag = 1, print to file
    //
    if (print_comment == 1)
    {
      fprintf(output_f_pointer, line); // print to output file

      // Print @param || @return to console
      if (strstr(line, "@return") != NULL)
      {
        // @RETURN
        char _returns[101 * sizeof(char)];
        parse_from(_returns, line, "@return");
        sprintf(method_desc[num_method_desc], "Returns:%s", _returns);

        num_method_desc++;
      }
      else if (strstr(line, "@param") != NULL)
      {
        // @PARAM
        char _param[101 * sizeof(char)];
        parse_from(_param, line, "@param");
        sprintf(method_desc[num_method_desc], "Parameter:%s", _param);

        num_method_desc++;
      }
    }

    //
    // When loop finds closing jdoc comment, end printing
    //
    if (strstr(prev_line, "*/") != NULL)
    {
      fprintf(output_f_pointer, "\n");

      // When prev_line = "*/" and line isn't class name
      if (strstr(line, "public class") == NULL)
      {
        parse_name(method_name, line, "("); // store new current method
        print_method = 1;                   // indicate method has changed, therefore print stored data
      }

      // Set printing flag to false
      print_comment = 0;
    }

    //
    // When method has finished being read
    // Print the data stored about method to console
    //
    if (print_method == 1)
    {
      // Print method name
      printf("Method: %s\n", method_name);
      // Print @param / @return
      for (int i = 0; i < num_method_desc; i++)
      {
        printf("%s\n", method_desc[i]);
      }
      printf("\n");

      print_method = 0;
      num_method_desc = 0;
    }

    // Store file class name
    if (strstr(line, "public class") != NULL)
      parse_name(class_name, line, "{"), print_class_author++;
    // Store file author
    if (strstr(line, "@author") != NULL)
      parse_from(file_author, line, "@author"), print_class_author++;
    // Print class name and author once ready
    if (print_class_author == 2)
    {
      printf("Class: %s \n", class_name);
      printf("Author:%s \n", file_author);
      printf("\n");

      print_class_author = 0;
    }

    if (is_empty_line(line) > 0)
      blank_lines++;         // when line is blank, increment blank_lines
    strcpy(prev_line, line); // store current line in previous line
    total_lines++;           // increment loop counter
  }

  // Close the input file once EOF
  fclose(input);
  fclose(output_f_pointer);

  // Print results
  printf("Total number of lines: %d \n", total_lines);
  printf("Number of non-blank lines: %d \n", total_lines - blank_lines);
  printf("Number of Javadoc comments: %d \n", num_comments);

  return 0;
}
