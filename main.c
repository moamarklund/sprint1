#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h> //toupper
#include <string.h>

struct storage_location{
  char character;
  int number;
};
typedef struct storage_location storage_location_t;

struct ware{
  char name[64];
  char description[128];
  storage_location_t storage_location;
  int price;
  int amount;
};
typedef struct ware ware_t;

enum flagtype{
  ADD,
  EDIT,
  REMOVE,
  UNCHANGED
};

struct db{
  int max_index;
  int current_index;
  ware_t *wares;
  ware_t copy_ware;
  int copy_index;
  int copy_flag;
};
typedef struct db db_t;

bool only_alpha(char str[]){
  int i = 0;
  while(str[i] != '\0'){
    if(isalpha(str[i]) == 0){
      const char exceptions[] = {' ',',','.'};
	//{'å', 'ä', 'ö', 'Å', 'Ä', 'Ö', ' ', ',', '.'};
      bool is_exception = false;
      for(int n=0; n<sizeof(exceptions); n++){
	if(str[i] == exceptions[n])
	  is_exception = true;
      }
      if(!is_exception)
	return false;
    }
    i++;
  }
  return true;
}

char ask_question_char(char message[]){
  bool have_answer = false;
  char input;
  while(!have_answer){
    printf("%s", message);
    input = toupper(getchar());
    while(getchar() != '\n');
    if(isalpha(input) != 0)
      have_answer = true;
    else
      printf("felaktigt svar, ange ett tecken.\n");
  }
  return input;
}

//0 not allowed
int ask_question_int(char message[]){
  bool have_answer = false;
  char buffer[sizeof(int)*8+1];
  while(!have_answer){
    printf("%s", message);
    gets(buffer);
    if(atoi(buffer) != 0)
      have_answer = true;
    else
      printf("felaktigt svar, ange en siffra.\n");
  }
  return atoi(buffer);
}

void ask_question_string(char message[], char return_message[]){
  bool have_answer = false;
  while(!have_answer){
    printf("%s", message);
    gets(return_message);
    if(only_alpha(return_message))
      have_answer = true;
    else
      printf("felaktigt svar, tillåtna tecken: a...ö , .\n");
  }
}

bool exit_program(){
  switch(ask_question_char("\nÄr du säker på att du vill avsluta?\n[J]a eller [N]ej _")){
  case 'J': return false; break;
  case 'N': return true; break;
  default:  return exit_program();
  }
}

void print_ware(ware_t *ware){
  printf("=================================\nVara:\t\t%s\nBeskrivning:\t%s\nPlats:\t\t%c%i\nPris:\t\t%i\nAntal:\t\t%i\n=================================\n", 
	 ware->name, ware->description, ware->storage_location.character, 
	 ware->storage_location.number, ware->price, ware->amount);
}

void db_insert(db_t *db, ware_t ware){
  if(db->current_index < db->max_index){
    db->copy_flag = ADD;
    db->copy_index = db->current_index;  
    db->wares[db->current_index++] = ware;
    print_ware(&db->wares[db->current_index-1]);
  }
  else
    puts("Fel, databasen är full");
}

//same as above but without stdout, unless error
void db_insert_mute(db_t *db, ware_t ware){
  if(db->current_index < db->max_index){
    db->copy_flag = ADD;
    db->copy_index = db->current_index;  
    db->wares[db->current_index++] = ware;
  }
  else
    puts("Fel, databasen är full");
}

//return true if location is occupied
int db_search_location(db_t *db, ware_t *ware){
  for(int c=0; c<db->current_index; c++){
    if(db->wares[c].storage_location.character == ware->storage_location.character
       && db->wares[c].storage_location.number == ware->storage_location.number){
      if(strcmp(db->wares[c].name, ware->name) == 0){
	db->wares[c].amount +=  ware->amount;
	return 1;
      }
      return 2;
    }
  }
  return 0;
}

//print wares from index start to index end, if possible
//return true if end reached
bool db_print(db_t *db, int start, int end){
  if(start < 0 || start > end)
    return false;
  
  while(start <= end){
    if(start >= db->current_index)
      return false;
    printf("%d.\t%s\n", start+1, db->wares[start].name);
    start++;
  }
  return true;
}

void add_goods(db_t *db){
  ware_t ware;
  printf("=================================\n");
  ask_question_string("Ange namnet på varan: ", ware.name);
  ask_question_string("Beskriv varan: ", ware.description);
  ware.price = ask_question_int("Ange priset för varan: ");
  ware.amount = ask_question_int("Ange antal: ");

  int status = 2;
  while(status == 2){
    ware.storage_location = (storage_location_t){.character=ask_question_char("Ange hylla: "),
						 .number=ask_question_int("Ange plats: ")};
    status = db_search_location(db, &ware);
      if(status == 2)
	puts("Platen är upptagen");
  }
 
  if(status != 1)
    db_insert(db, ware);
}

void remove_goods_aux(db_t *db, int index){
  if(db->current_index == index+1){
    db->copy_flag = REMOVE;
    db->current_index--;
    db->copy_index = db->current_index;
  }
  else{
    db->copy_flag = REMOVE;
    db->copy_ware = db->wares[index];
    db->copy_index = index;

    for(int c=index; c<db->current_index; c++){
      db->wares[c] = db->wares[c+1];
    }
    db->current_index--;
  } 
}

void remove_goods(db_t *db){
  remove_goods_aux(db, ask_question_int("Ange ett nummer: _")-1);
}

void edit_goods(db_t *db){
  bool running = true;
  int value = ask_question_int("Ange numret på den vara du vill ändra:_ ");
  print_ware(&db->wares[value-1]);

  db->copy_flag = EDIT;
  db->copy_ware = db->wares[value-1];
  db->copy_index = value-1;

  while(running){
    char answer = ask_question_char("Välj vad du vill redigera\n[N]amn\n[B]eskrivning\n[P]ris\n[L]agerhylla\nAn[t]al\neller [A]vbryt:_");
    switch(answer){
    case 'N':
      printf("Nuvarande namn: %s\n", db->wares[value-1].name);
      ask_question_string("Nytt namn:_", db->wares[value-1].name);
      break;
    case 'B':
      printf("Nuvarande beskrivning: %s\n", db->wares[value-1].description);
      ask_question_string("Ny beskrivning:_", db->wares[value-1].description);
      break;
    case 'P':
      printf("Nuvarande pris: %i\n", db->wares[value-1].price);
      db->wares[value-1].price = ask_question_int("Nytt pris:_");
      break;
    case 'L':
      printf("Nuvarande lagerhylla: %c%i\n", db->wares[value-1].storage_location.character, db->wares[value-1].storage_location.number);

      int status = 2;
      while(status == 2){
	db->wares[value-1].storage_location = (storage_location_t){.character=ask_question_char("Ange hylla: "),
								   .number=ask_question_int("Ange plats: ")};
	status = db_search_location(db, &db->wares[value-1]);
	if(status == 2){
	  puts("Platsen är upptagen");
	}
	else if(status == 1){
	  puts("Varan finns redan på den platsen, antalet ökat.");
	  remove_goods_aux(db, (value-1));
	}
      }
      break;
	
    case 'T':
      printf("Nuvarande antal: %i\n", db->wares[value-1].amount);
      db->wares[value-1].amount = ask_question_int("Nytt antal:_");
      break;
    case 'A': running = false;
      break;
    }
  }
}

void undo_goods(db_t *db){
  if(db->copy_flag == UNCHANGED){
    puts("Du måste göra något innan du kan ångra.");
  }
  else if(db->copy_flag == ADD){
    printf("Ångrade %s.\n", db->wares[db->copy_index].name);
    remove_goods_aux(db, db->copy_index);
  }
  else if(db->copy_flag == EDIT){
    printf("Ångrade %s.\n", db->wares[db->copy_index].name);
    db->wares[db->copy_index] = db->copy_ware;
  }
  else if(db->copy_flag == REMOVE){
    printf("Ångrade %s.\n", db->copy_ware.name);
    if(db->copy_flag == db->current_index)
      db->current_index++;
    else{
      for(int c=db->current_index; c>db->copy_index; c--){
	db->wares[c] = db->wares[c-1];
      }
      db->wares[db->copy_index] = db->copy_ware;
      db->current_index++;
    }
  }

  db->copy_flag = UNCHANGED;
}

void list_goods(db_t *db){
  bool running = true;
  int index = 0;
  char answer;
  bool next_page_exists;
  while(running){
    next_page_exists = db_print(db, index, index + 19);    
    
    answer = ask_question_char("=================================\n[V]älj ett nummer för att visa mer information om varan.\nVisa [m]er\n[A]vsluta.\n_ ");
    
    if(answer == 'A'){
      running = false;
    }
    else if(answer == 'M'){
      if(next_page_exists)
	index += 20;
      else
	puts("Finns ej fler varor");
    }
    else if(answer == 'V'){
      int value = ask_question_int("Ange ett nummer _");
      print_ware(&db->wares[value-1]);
    }
  }
}

void db_autofill(db_t *db, int items){
  const char *names[] = {"Bullar", "Citron", "Deg", "Fisk", "Gurka", "Ingefära", "Jordnötter",
			 "Kaffe", "Lingon", "Mjölk", "Nudlar", "Päron", "Smör", "Tomat", NULL};
  int names_s = -1;
  while(names[++names_s]);
  const char desc[] = "Blah blah ";
  int names_usage[sizeof(names)/sizeof(char)];
  const int amounts[] = {100, 200, 300, 400, 1337};
  const int price_divider = 20;
  int c;
  for(c=0; c<names_s; c++){
    names_usage[c] = 1;
  }

  for(c=0; c<items; c++){
    ware_t ware;
    strcpy(ware.name, names[c%names_s]);
    strcpy(ware.description, desc);
    strcat(ware.description, names[c%names_s]);
    ware.amount=amounts[c%(sizeof(amounts)/sizeof(int))];
    ware.price=(amounts[c%(sizeof(amounts)/sizeof(int))]/price_divider);
    ware.storage_location=(storage_location_t){.character=*names[c%names_s],
	                                       .number=names_usage[c%names_s]++};
    db_insert_mute(db, ware);
  }
}

int main(int argc, char *argv[]){
  bool running = true;
  char input;
  ware_t wares[1024];
  db_t db = {.current_index=0, .max_index=sizeof(wares)/sizeof(ware_t), .wares=wares, .copy_index=-1};

  db_autofill(&db, 10);

  while(running){     
    input = ask_question_char("\nVälkommen till lagerhantering 1.0\n=================================\n[L]ägga till en vara\n[T]a bort en vara\n[R]edigera en vara\nÅn[g]ra senaste ändringen\nLista [h]ela varukatalogen\n[A]vsluta\n\nVad vill du göra idag? _");

    switch(input){
    case 'L': add_goods(&db); break;
    case 'T': remove_goods(&db); break;
    case 'R': edit_goods(&db); break;
    case 'G': undo_goods(&db); break;
    case 'H': list_goods(&db); break;
    case 'A': running = exit_program(); break;
    default: puts("Okänt kommando.");
    }
  }
  return 0;
}
