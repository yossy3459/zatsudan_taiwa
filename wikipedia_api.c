#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

#define BUF_SIZE 1024
#define PAGEBUF_SIZE 16384
#define SERVER_PORT 80
#define PROXY_PORT 10080

/* Webプロキシサーバを使用するかどうか */
#define USE_PROXY 1

/* Wikipediaを利用して単語の意味を得る */
char *get_Wikipedia(char* word) {
  int sockfd; /* ソケットのためのファイルディスクリプタ */
  struct hostent *servhost; /* ホスト名とIPアドレスを扱うための構造体 */
  struct sockaddr_in server; /* ソケットを扱うための構造体 */
  char *host = "wikipedia.simpleapi.net"; /* 接続するホスト名 */
  char *path = "/api"; /* 要求するパス */
  char *proxy_host = "proxy.sic.shibaura-it.ac.jp"; /* プロキシサーバ（学内） */

  char send_buf[BUF_SIZE]; /* サーバに送るHTTPプロトコル用バッファ */
  char page_buf[PAGEBUF_SIZE]; /* サーバからの応答の保存用バッファ */
  char *ret; /* この関数の返り値となる，サーバ応答文字列へのポインタを格納する */
  int count, i;

  /* ホスト情報servhostの取得 */
  if (USE_PROXY) {
    servhost = gethostbyname(proxy_host);
  } else {
    servhost = gethostbyname(host);
  }
  if (servhost == NULL) {
    herror("client: gethostbyname");
    exit(1);
  }

  /* ソケットを扱うための構造体serverの設定 */
  bzero((char *)&server, sizeof(server));
  server.sin_family = PF_INET;
  if (USE_PROXY) {
    server.sin_port = htons(PROXY_PORT);
  } else {
    server.sin_port = htons(SERVER_PORT);
  }
  bcopy(servhost->h_addr, (char *)&server.sin_addr, servhost->h_length);

  /* ソケットの生成 */
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("client: socket");
    exit(1);
  }

  /* サーバに接続する */
  if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    perror("client: connect");
    exit(1);
  }

  /* HTTPプロトコルの送信 */
  send_message(sockfd, "GET "); /* GETメソッドを使う */
  if (USE_PROXY) {
    send_message(sockfd, "http://");
    send_message(sockfd, host);
  }
  send_message(sockfd, path);

  send_message(sockfd, "?keyword="); /* 検索単語をURLエンコードしてパスに付加する */
  send_message(sockfd, url_encode(word));
  //  send_message(sockfd, "?pn=p4"); /* その他のパラメタ値も付加 */

  send_message(sockfd, " HTTP/1.0\r\n"); /* 使用するHTTPのバージョン（1.1でもよい） */
  send_message(sockfd, "HOST: "); /* ホスト名の指定 */
  if (USE_PROXY) {
    send_message(sockfd, proxy_host);
  } else {
    send_message(sockfd, host);
  }
  send_message(sockfd, "\r\n");

  send_message(sockfd, "\r\n"); /* 空行を送って送信終了 */

  /* サーバからの応答を受信する */
  count = 0; /* 受信したバイト数 */
  while (1) {
    char buf[BUF_SIZE];
    int len;
    len = read(sockfd, buf, BUF_SIZE); /* ソケットから文字列を受信してbufに格納する */
    if (len > 0) {
      for(i=0; i<len; i++) {
	page_buf[count++] = buf[i];
      }
    } else {
      page_buf[count++] = '\0';
      break;
    }
  }
  ret = (char *)malloc(count);
  for (i = 0; i < count; i++) {
    ret[i] = page_buf[i];
  }

  close(sockfd); /* ソケットを閉じる */
  return ret;
}

char *mecab_wikipedia(char input[])
{
  char *syutugen, *hinsi1, *hinsi2, *hinsi3, *hinsi4;
  char *katuyo1, *katuyo2, *kihon;

  FILE *fp1,*fp2;
  char buf[1024],temp[1024]={},send[1024]={};

  if ( (fp1 = fopen("temp_wiki_zatudan1.txt","w")) == NULL ){
    printf("file open failure.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(fp1,"%s",input);
  fclose(fp1);

  system("mecab temp_wiki_zatudan1.txt > temp_wiki_zatudan2.txt");

  if ( (fp2 = fopen("temp_wiki_zatudan2.txt","r")) == NULL ){
    printf("file open failure.\n");
    exit(EXIT_FAILURE);
  }

  while (fgets(buf, 1024, fp2) != NULL) {
    //      printf("%s", buf);
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

     // 最後が名詞となるように整形

    if ( strcmp(hinsi1,"名詞") )
      strcat(temp,syutugen);
    else{
      strcat(send,temp);
      strcat(send,syutugen);
      strcpy(temp,"");
    }

  }

  input = send;
  return input;
}


/* main関数 */
char* mainWikipedia(char *word) {
  char *result, *p, *q, *place_of_colon;
  static char buf[1024];
  static char send[1024];
  int i,num;
  bool colon = false;


  // 自作: wikipedia の body から 最初の一文を抽出

  result = get_Wikipedia(word);
   printf("%s\n",result);

  p = result; /* ポインタpの初期化 */

  while ( (p = strstr(p,"<") ) != NULL ) {

    if ( strstr(p, "<body>") == p ) { // 最初の <body> タグ
      p += 6; // "<body>" の文字数
      q = strstr(p, "。") + 3; // "。" までが一文
      
      for (i = 0; p + i < q; i++) { // カンマ間の文字列を抽出
	buf[i] = *(p + i);
	if ( *(p+i) == ':' ){
     	  //colon = true;
	  //place_of_colon = (p + i + 1);
	  //	  break;
	}
      }
      
      if ( colon == true ){
	for ( i=0 ; place_of_colon + i < q ; i++ )
	  buf[i] = *(place_of_colon + i);
      }
      
      sprintf(send,"%s", mecab_wikipedia(buf));
      
      return send;
    }
    p++;
  }
}


