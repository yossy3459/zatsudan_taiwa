/* README:
  */

#include "zatudan_final.h"

/* 文字列の複製を作って返す */
char *duplicate(char *s) {
  char *ret;
  int i;

  ret = (char *)malloc(strlen(s) + 1);
  for (i = 0; s[i] != '\0'; i++) {
    ret[i] = s[i];
  }
  ret[i] = '\0';

  return ret;
}

/* 反応パターンデータの初期化を行う */
void setup_reaction_pattern() {
  char buf[256];
  char kw[256], response[256];
  int i;
  FILE *fp;

  n_pattern = 0;
  if ((fp = fopen("pattern.txt", "r")) == NULL) {
    fprintf(stderr, "can't open pattern.txt\n");
    exit(1);
  }
  while (fgets(buf, 256, fp) != NULL) {
    sscanf(buf, "%s\t%s", kw, response);
    pattern[n_pattern].kw = duplicate(kw);
    pattern[n_pattern].response = duplicate(response);
    n_pattern++;
  }
}

// 自作 : 形容詞検出後、ユーザーの反応にたいして「そうなんだ！」と返す
void generate_response_after_adjective(){
  double score = 5.0;

  if ( global_max_score == 3.0 ){
    candidate[n_candidate].response = "そうなんだ！";
    candidate[n_candidate].score = score;
    n_candidate++;
    global_max_score == 0.0;
  }
}

/* 反応パターンを利用した応答候補の生成 */
void generate_response_by_pattern(char *input) {
  double score = 4.0;
  int i;

  for (i = 0; i < n_pattern; i++) {
    if (strstr(input, pattern[i].kw) != NULL) {
	candidate[n_candidate].response = pattern[i].response;
	candidate[n_candidate].score = score;
	n_candidate++;
    }
  }
}

// 自作 : 形容詞を検出してどの程度か聞く応答分を作成(mecab利用)
void generate_response_by_adjective(char *input){
  double score = 3.0;
  char *syutugen, *hinsi1, *hinsi2, *hinsi3, *hinsi4;
  char *katuyo1, *katuyo2, *kihon;

  FILE *fp1,*fp2;
  char buf[1024],adj[128];

  if ( (fp1 = fopen("zatudan1.txt","w")) == NULL ){
    printf("file open failure.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(fp1,"%s",input);
  fclose(fp1);

  system("mecab zatudan1.txt > zatudan2.txt");

  if ( (fp2 = fopen("zatudan2.txt","r")) == NULL ){
    printf("file open failure.\n");
    exit(EXIT_FAILURE);
  }

  while (fgets(buf, 1024, fp2) != NULL) {
    /* printf("%s", buf); */
    if (strcmp(buf, "EOS\n") == 0) { /* 文の終わりを示す記号 */
      continue;
    }

    syutugen = strtok(buf, "\t"); /* 出現形 */
    hinsi1 = strtok(NULL, ","); /* 品詞 第1階層 */
    hinsi2 = strtok(NULL, ","); /* 品詞 第2階層 */
    hinsi3 = strtok(NULL, ","); /* 品詞 第3階層 */
    hinsi4 = strtok(NULL, ","); /* 品詞 第4階層 */
    katuyo1 = strtok(NULL, ","); /* 活用型 */
    katuyo2 = strtok(NULL, ","); /* 活用形 */
    kihon = strtok(NULL, ","); /* 基本形（終止形） */
    //  printf("%s\n",hinsi1);


    if ( !strcmp(hinsi1,"形容詞") ){
      sprintf(adj,"%sってどのくらい%sの？",kihon,kihon);
      candidate[n_candidate].response = adj;
      candidate[n_candidate].score = score;
      n_candidate++;
    }
  }
}


/* それ以外の応答候補生成 -> Google Trend より*/
void generate_conversation_from_google_trend(char *trend[]) {
  char res_temp[256];

  double score = 1.0;

  if ( trend_num < TREND_MAX ){
    sprintf(res_temp,"今、「%s」が流行っているんだって!",trend[trend_num]);
    candidate[n_candidate].response = res_temp;
    candidate[n_candidate].score = score;
    n_candidate++;
  }
}


// 自作 : 名詞を検出してwikipedia検索(mecab利用)
void generate_response_by_a_proper_noun(char *input){
  double score = 2.0;
  char *syutugen, *hinsi1, *hinsi2, *hinsi3, *hinsi4;
  char *katuyo1, *katuyo2, *kihon;

  FILE *fp1,*fp2;
  char buf[1024];
  char noun[1024];
  char result[4096] = {};

  if ( (fp1 = fopen("temp_wiki_zatudan3.txt","w")) == NULL ){
    printf("file open failure.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(fp1,"%s",input);
  fclose(fp1);

  system("mecab temp_wiki_zatudan3.txt > temp_wiki_zatudan4.txt");

  if ( (fp2 = fopen("temp_wiki_zatudan4.txt","r")) == NULL ){
    printf("file open failure.\n");
    exit(EXIT_FAILURE);
  }

  while (fgets(buf, 1024, fp2) != NULL) {
    //printf("%s", buf);
    if (strcmp(buf, "EOS\n") == 0) { /* 文の終わりを示す記号 */
      continue;
    }

    syutugen = strtok(buf, "\t"); /* 出現形 */
    hinsi1 = strtok(NULL, ","); /* 品詞 第1階層 */
    hinsi2 = strtok(NULL, ","); /* 品詞 第2階層 */
    hinsi3 = strtok(NULL, ","); /* 品詞 第3階層 */
    hinsi4 = strtok(NULL, ","); /* 品詞 第4階層 */
    katuyo1 = strtok(NULL, ","); /* 活用型 */
    katuyo2 = strtok(NULL, ","); /* 活用形 */
    kihon = strtok(NULL, ","); /* 基本形（終止形） */
    //printf("%s\n",hinsi1);

    if ( !strcmp(hinsi2,"固有名詞") ){
      strcpy(noun,syutugen);
      // printf("%s",noun);
      sprintf(result,"%sのことだよ。",mainWikipedia(noun));
      candidate[n_candidate].response = result;
      candidate[n_candidate].score = score;
      n_candidate++;
    }
  }
}



/* 1つ前の自分の発言からの話題展開による応答候補の生成 */
void generate_response_by_expansion(char *previous_output) {
  double score = 0.5;

  /* この例では単に繰り返すだけ */
  candidate[n_candidate].response = previous_output;
  candidate[n_candidate].score = score;
  n_candidate++;
}


/* 応答文を決める */
char *generate_response(char *input, char *previous_output, char *trend[]) {
  char *ret = NULL;
  double max_score = -1.0;
  int i,j;

  /* 複数の方法での応答候補生成を行う */
  n_candidate = 0;
  generate_response_after_adjective();
  generate_response_by_pattern(input);
  generate_response_by_adjective(input);
  generate_conversation_from_google_trend(trend);
  generate_response_by_a_proper_noun(input);
  generate_response_by_expansion(previous_output);
  //generate_response_by_a_noun(input);


  // デバッグ用
  // for (j = 0; j < n_candidate; j++)
  //printf("%f\n",candidate[j].score);


  /* スコア最大の応答候補を選択する */
  for (i = 0; i < n_candidate; i++) {
    if (candidate[i].score > max_score) {
      ret = candidate[i].response;
      max_score = candidate[i].score;
      // 自作用処理
      global_max_score = candidate[i].score;  // score 5.0用
      if (max_score == 1.0){
	trend_num++;
      }
    }
  }

  return ret;
}

/* 雑談対話プログラムのメイン関数の例 */
int main() {
  char input[1024];
  char *output = "こんにちは！";
  char *nickname = "yossy   "; // この対話システムの呼び名
  FILE *fp;
  int i;
  char *trend[TREND_MAX];

  trend_num = 0;

  // グローバル変数 trend の領域確保
  for ( i=0 ; i<TREND_MAX ; i++ )
    trend[i] = (char *)malloc(128 * sizeof(char));

  // Trends の Database を読み込みモードでオープン
  if ( (fp = fopen("trends_database.txt","r")) == NULL ){
    printf("Error: \"trends_database.txt\" could not open.\n");
    exit(1);
  }

  // Google Trends API を読み込み、 trends_databaseへ格納。
  mainGoogleTrendsApi();

  // ポインタの配列 trend へ Database の内容を流し込む
  for ( i=0 ; i<TREND_MAX ; i++ ){
    fscanf(fp,"%s",trend[i]);
    // printf("%s\n",trend[i]);
  }

  // ファイルクローズ
  fclose(fp);

  /* 反応パターンデータの初期化 */
  setup_reaction_pattern();

  /* 最初の挨拶 */
  printf("%s：%s\n", nickname, output);
  fflush(stdout);

  while (1) {
    /* ユーザからの入力を受け付ける */
    printf("ユーザ　：");
    fflush(stdout);
    scanf("%s", &input);

    /* システムの応答を生成し出力する */
    output = generate_response(input, output, trend);
    printf("%s：%s\n", nickname, output);
    fflush(stdout);

    if ( trend_num == 7 ){

      trend_num = 0;

      // グローバル変数 trend の領域確保
      for ( i=0 ; i<TREND_MAX ; i++ )
	trend[i] = (char *)malloc(128 * sizeof(char));

      // Trends の Database を読み込みモードでオープン
      if ( (fp = fopen("trends_database.txt","r")) == NULL ){
	printf("Error: \"trends_database.txt\" could not open.\n");
	exit(1);
      }

      // Google Trends API を読み込み、 trends_databaseへ格納。
      mainGoogleTrendsApi();

      // ポインタの配列 trend へ Database の内容を流し込む
      for ( i=0 ; i<TREND_MAX ; i++ ){
	fscanf(fp,"%s",trend[i]);
    // printf("%s\n",trend[i]);
      }

      // ファイルクローズ
      fclose(fp);
    }
  }

  free(trend);
}
