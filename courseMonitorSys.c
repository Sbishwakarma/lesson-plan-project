#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define ENTER 10
#define BACKSPACE 127

struct user
{
	char fullname[50];
	char username[50];
	char email[50];
	char password[50];
	char role[50];
};

struct course
{
	char id[20];
	char name[50];
	char teacher[50];
	char lessons[500];		   // comma-separated lesson names
	float lessonCoverages[50]; // individual lesson percentages
	int lessonCount;		   // count of lessons
	float totalCoverage;	   // aggregated percentage of all lessons
};

// void takeInput(char input[50])
// {
// 	fgets(input, 50, stdin);
// 	input[strlen(input) - 1] = 0;
// };

void takeInput(char *input, size_t size)
{
	fgets(input, size, stdin);
	input[strlen(input) - 1] = 0;
};

void generateUsername(char email[50], char username[50])
{
	for (int i = 0; i < strlen(email); i++)
	{
		if (email[i] == '@')
		{
			break;
		}
		else
		{
			username[i] = email[i];
		};
	};
};

// Function to emulate getch()
int getch(void)
{
	struct termios oldattr, newattr;
	int ch;
	tcgetattr(STDIN_FILENO, &oldattr);
	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
	return ch;
}

void takepassword(char pwd[50])
{
	int i = 0;
	char ch;
	while (1)
	{
		ch = getch();
		if (ch == ENTER)
		{
			pwd[i] = '\0';
			break;
		}
		else if (ch == BACKSPACE)
		{
			if (i > 0)
			{
				i--;
				printf("\b \b");
			}
		}
		else
		{
			pwd[i++] = ch;
			printf("*");
		}
	}
};

// Function to clear the screen on Linux
void clearScreen()
{
	printf("\033[2J");
	printf("\033[H");
}

// Function to produce a beep sound (approximate)
void beep()
{
	printf("\a");
	fflush(stdout);
}

void createCourse()
{
	struct course newCourse;
	FILE *fp = fopen("Courses.dat", "a+");

	printf("\nEnter Course ID: ");
	takeInput(newCourse.id, sizeof(newCourse.id));
	printf("Enter Course Name: ");
	takeInput(newCourse.name, sizeof(newCourse.name));
	printf("Enter Teacher Username: ");
	takeInput(newCourse.teacher, sizeof(newCourse.teacher));
	printf("Enter List of Lessons (comma-separated): ");
	takeInput(newCourse.lessons, sizeof(newCourse.lessons));

	// Split lessons and initialize coverages
	char tempLessons[500];
	strcpy(tempLessons, newCourse.lessons);
	char *lesson = strtok(tempLessons, ",");
	newCourse.lessonCount = 0;
	while (lesson)
	{
		newCourse.lessonCoverages[newCourse.lessonCount++] = 0.0;
		lesson = strtok(NULL, ",");
	}

	newCourse.totalCoverage = 0.0;

	fwrite(&newCourse, sizeof(struct course), 1, fp);
	printf("\nCourse created successfully!\n");
	fclose(fp);
}

void listCourses()
{
	struct course crs;
	FILE *fp = fopen("Courses.dat", "r");

	printf("\nList of Courses:\n");
	while (fread(&crs, sizeof(struct course), 1, fp))
	{
		printf("\nID: %s\nName: %s\nTeacher: %s\nLessons:\n", crs.id, crs.name, crs.teacher);
		char lessons[500];
		strcpy(lessons, crs.lessons);
		char *lesson = strtok(lessons, ",");
		for (int i = 0; i < crs.lessonCount && lesson; i++)
		{
			printf("  - %s: %.2f%%\n", lesson, crs.lessonCoverages[i]);
			lesson = strtok(NULL, ",");
		}
		printf("Total Coverage: %.2f%%\n", crs.totalCoverage);
	}
	fclose(fp);
}

void deleteCourse()
{
	char courseId[20];
	struct course crs;
	FILE *fp = fopen("Courses.dat", "r");
	FILE *temp = fopen("Temp.dat", "w");

	printf("\nEnter Course ID to delete: ");
	takeInput(courseId, sizeof(courseId));

	int found = 0;
	while (fread(&crs, sizeof(struct course), 1, fp))
	{
		if (strcmp(crs.id, courseId) == 0)
		{
			found = 1;
			printf("\nCourse with ID %s deleted successfully!\n", courseId);
		}
		else
		{
			fwrite(&crs, sizeof(struct course), 1, temp);
		}
	}

	fclose(fp);
	fclose(temp);

	remove("Courses.dat");
	rename("Temp.dat", "Courses.dat");

	if (!found)
	{
		printf("\nCourse ID not found!\n");
	}
}

void updateCoverage(char username[50])
{
	char courseId[20];
	struct course crs;
	FILE *fp = fopen("Courses.dat", "r+");

	printf("\nEnter Course ID to update: ");
	takeInput(courseId, sizeof(courseId));

	int found = 0;
	while (fread(&crs, sizeof(struct course), 1, fp))
	{
		if (strcmp(crs.id, courseId) == 0 && strcmp(crs.teacher, username) == 0)
		{
			printf("\nLessons and Current Coverage:\n");
			char lessons[500];
			strcpy(lessons, crs.lessons);
			char *lesson = strtok(lessons, ",");
			for (int i = 0; i < crs.lessonCount && lesson; i++)
			{
				printf("%d. %s: %.2f%%\n", i + 1, lesson, crs.lessonCoverages[i]);
				lesson = strtok(NULL, ",");
			}

			int lessonIndex;
			printf("\nEnter lesson number to update (1-%d): ", crs.lessonCount);
			scanf("%d", &lessonIndex);
			fgetc(stdin);

			if (lessonIndex > 0 && lessonIndex <= crs.lessonCount)
			{
				float newCoverage;
				printf("Enter new coverage percentage for lesson %d: ", lessonIndex);
				scanf("%f", &newCoverage);
				fgetc(stdin);

				crs.lessonCoverages[lessonIndex - 1] = newCoverage;

				// Recalculate total coverage
				float total = 0.0;
				for (int i = 0; i < crs.lessonCount; i++)
				{
					total += crs.lessonCoverages[i];
				}
				crs.totalCoverage = total / crs.lessonCount;

				fseek(fp, -sizeof(struct course), SEEK_CUR);
				fwrite(&crs, sizeof(struct course), 1, fp);
				printf("\nLesson coverage updated successfully!\n");
			}
			else
			{
				printf("\nInvalid lesson number!\n");
			}

			found = 1;
			break;
		}
	}

	if (!found)
	{
		printf("\nYou are not authorized for this course or Course ID not found!\n");
	}

	fclose(fp);
}

void assignNewTeacher()
{
	char courseId[20], newTeacher[50];
	struct course crs;
	FILE *fp = fopen("Courses.dat", "r+");

	printf("\nEnter Course ID to update teacher: ");
	takeInput(courseId, sizeof(courseId));
	printf("Enter new teacher's username: ");
	takeInput(newTeacher, sizeof(newTeacher));

	int found = 0;
	while (fread(&crs, sizeof(struct course), 1, fp))
	{
		if (strcmp(crs.id, courseId) == 0)
		{
			strcpy(crs.teacher, newTeacher);
			fseek(fp, -sizeof(struct course), SEEK_CUR);
			fwrite(&crs, sizeof(struct course), 1, fp);
			printf("\nTeacher updated successfully for course ID: %s\n", courseId);
			found = 1;
			break;
		}
	}

	if (!found)
	{
		printf("\nCourse ID not found!\n");
	}

	fclose(fp);
}

void listTeachers()
{
	struct user usr;
	FILE *fp = fopen("Users.dat", "r");

	printf("\nList of Teachers:\n");
	while (fread(&usr, sizeof(struct user), 1, fp))
	{
		if (strcmp(usr.role, "Teacher") == 0)
		{
			printf("\nFull Name: %s\nUsername: %s\nEmail: %s\n", usr.fullname, usr.username, usr.email);
		}
	}

	fclose(fp);
}

void deleteTeacher()
{
	char teacherUsername[50];
	struct user usr;
	FILE *fp = fopen("Users.dat", "r");
	FILE *temp = fopen("Temp.dat", "w");

	if (fp == NULL || temp == NULL)
	{
		printf("\nError opening file!\n");
		return;
	}

	printf("\nEnter the username of the teacher to delete: ");
	takeInput(teacherUsername, sizeof(teacherUsername));

	int found = 0;
	while (fread(&usr, sizeof(struct user), 1, fp))
	{
		if (strcmp(usr.username, teacherUsername) == 0 && strcmp(usr.role, "Teacher") == 0)
		{
			found = 1;
			printf("\nTeacher with username '%s' deleted successfully!\n", teacherUsername);
		}
		else
		{
			fwrite(&usr, sizeof(struct user), 1, temp);
		}
	}

	fclose(fp);
	fclose(temp);

	remove("Users.dat");
	rename("Temp.dat", "Users.dat");

	if (!found)
	{
		printf("\nTeacher with username '%s' not found!\n", teacherUsername);
	}
}

int main()
{

	FILE *fp;
	int opt, usrFound = 0;
	char password2[50];
	struct user user;

	printf("\n\t\t\t\t--------------------welcome to lesson monitor system--------------------");
	printf("\nPlease choose your option");
	printf("\n1.Signup");
	printf("\n2.Login");
	printf("\n3.Exit");

	printf("\n\nYour choice:\t");
	scanf("%d", &opt);
	fgetc(stdin);

	switch (opt)
	{
	case 1:
		clearScreen();
		printf("\nEnter your full name:\t");
		takeInput(user.fullname, sizeof(user.fullname));
		printf("Enter your role:\t");
		takeInput(user.role, sizeof(user.role));
		printf("Enter your email:\t");
		takeInput(user.email, sizeof(user.email));
		printf("Enter your password:\t");
		takepassword(user.password);
		printf("\nConfirm your password:\t");
		takepassword(password2);

		if (!strcmp(user.password, password2))
		{
			generateUsername(user.email, user.username);
			fp = fopen("Users.dat", "a+");
			size_t elements_written = fwrite(&user, sizeof(struct user), 1, fp);
			if (elements_written == 1)
				printf("\n\nUser registration success, Your username is %s\n", user.username);
			else
				printf("\n\nSorry! Something went wrong :(\n");
			fclose(fp);
		}
		else
		{
			printf("\n\nPasswords do not match\n");
			beep();
			sleep(1); // Pause for 1 second
		}
		break;
	case 2:

		char username[50], pword[50];
		struct user usr;

		printf("\nEnter your username:\t");
		takeInput(username, sizeof(username));
		printf("Enter your password:\t");
		takepassword(pword);

		fp = fopen("Users.dat", "r");
		while (fread(&usr, sizeof(struct user), 1, fp))
		{
			if (!strcmp(usr.username, username))
			{
				if (!strcmp(usr.password, pword))
				{
					clearScreen();
					printf("\n\t\t\t\t\t\tWelcome %s", usr.fullname);
					printf("\n\n|Full Name:\t%s", usr.fullname);
					printf("\n|Email:\t\t%s", usr.email);
					printf("\n|Username:\t%s\n", usr.username);

					if (strcmp(usr.fullname, "HOD") == 0)
					{
						int hodOpt;
						do
						{
							printf("\nHOD Menu:\n1. Create Course\n2. List Courses\n3. Delete Course\n4. Assign New Teacher\n5. List Teachers\n6. Delete Teacher\n7. Logout\nYour choice: ");
							scanf("%d", &hodOpt);
							fgetc(stdin);

							switch (hodOpt)
							{
							case 1:
								createCourse();
								break;
							case 2:
								listCourses();
								break;
							case 3:
								deleteCourse();
								break;
							case 4:
								assignNewTeacher();
								break;
							case 5:
								listTeachers();
								break;
							case 6:
								deleteTeacher();
								break;
							case 7:
								printf("\nLogging out...\n");
								break;
							default:
								printf("\nInvalid choice!\n");
							}
						} while (hodOpt != 7);
					}
					else
					{
						int userOpt;
						do
						{
							printf("\nUser Menu:\n1. View Assigned Courses\n2. Update Lesson Coverage\n3. Logout\nYour choice: ");
							scanf("%d", &userOpt);
							fgetc(stdin);

							switch (userOpt)
							{
							case 1:
								listCourses(); // You may want to filter courses based on the teacher's username.
								break;
							case 2:
								updateCoverage(usr.username);
								break;
							case 3:
								printf("\nLogging out...\n");
								break;
							default:
								printf("\nInvalid choice!\n");
							}
						} while (userOpt != 3);
					}
					usrFound = 1;
					break;
				}
				else
				{
					printf("\nIncorrect password!\n");
					beep();
				}
			}
		}
		fclose(fp);

		if (!usrFound)
		{
			printf("\nUser not found!\n");
		}
		break;
	case 3:
		printf("\nExiting the system. Goodbye!\n");
		break;
	default:
		printf("\nInvalid option!\n");
	}
	return 0;
}