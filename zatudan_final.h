#ifndef ZATUDAN_FINAL_H
#define ZATUDAN_FINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 反応パターンの最大数 */
#define PATTERN_MAX 100
/* 応答候補の最大数 */
#define CANDIDATE_MAX 100

// 自作: Trend の最大数
#define TREND_MAX 7

/* 反応パターンを表す構造体 */
struct reaction_pattern {
  char *kw; /* 反応の基となるキーワード */
  char *response; /* そのキーワードに対する応答文 */
};
/* 応答候補を表す構造体 */
struct response_candidate {
  char *response; /* 応答候補文 */
  double score; /* その望ましさを表すスコア */
};

/* 反応パターンを格納する配列 */
struct reaction_pattern pattern[PATTERN_MAX];
/* 応答候補を格納する配列 */
struct response_candidate candidate[CANDIDATE_MAX];

/* 反応パターンの数を表す変数 */
int n_pattern;
/* 現時点の応答候補数を表す変数 */
int n_candidate;

// 自作
double global_max_score;
static int trend_num;

// プロトタイプ宣言
char *duplicate(char *);
void setup_reaction_pattern();
void generate_response_after_adjective();
void generate_response_by_pattern(char *);
void generate_response_by_adjective(char *);
void generate_conversation_from_google_trend(char *[]);
void generate_response_by_a_proper_noun(char *);
void generate_response_by_expansion(char *);
char *generate_responce(char *, char *, char *);

#endif
