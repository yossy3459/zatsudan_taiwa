#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define PAGEBUF_SIZE 16384
#define SERVER_PORT 80
#define PROXY_PORT 10080

/* Webプロキシサーバを使用するかどうか */
#define USE_PROXY 1

/* URLエンコードを行う（sに半角文字が含まれないと仮定した簡略版） */
char *url_encode(char *s) {
  int len, i;
  char *ret;

  len = strlen(s);
  ret = (char *)malloc(sizeof(char) * (len * 3 + 1));
  for (i = 0; i < len; i++) {
    sprintf(ret + i * 3, "%%%02X", (unsigned char)s[i]);
  }
  ret[len*3] = '\0';

  return ret;
}

/* ソケットに文字列メッセージを送信する */
char send_message(int sockfd, char *msg) {
  if (write(sockfd, msg, strlen(msg)) == -1) {
    perror("client: write");
    exit(1);
  }
}

/* Google Trends APIを利用して現在のトレンドを得る */
char *get_GoogleTrends() {
  int sockfd; /* ソケットのためのファイルディスクリプタ */
  struct hostent *servhost; /* ホスト名とIPアドレスを扱うための構造体 */
  struct sockaddr_in server; /* ソケットを扱うための構造体 */
  char *host = "trends.google.co.jp"; /* 接続するホスト名 */
  char *path = "/trends/hottrends/atom/hourly"; /* 要求するパス */
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

  //  send_message(sockfd, "?"); /* 検索単語をURLエンコードしてパスに付加する */
  //  send_message(sockfd, url_encode(word));
  send_message(sockfd, "?pn=p4"); /* その他のパラメタ値も付加 */

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

/* main関数 */
void mainGoogleTrendsApi() {
  char *result, *tag, *p, *q;
  char buf[1024];
  int i, count;
  FILE *fp;

  if ( (fp = fopen("trends_database.txt","w")) == NULL ){
    printf("File Open Error!/n");
    exit(1);
  }
 
  /* Google Trends APIを利用して、現在のトレンドを含む応答を得る */
  result = get_GoogleTrends();



  // 自作: トレンドの上位7件を抽出し、外部データベースに保存  

  p = result; /* ポインタpの初期化 */
 
  if ( (p = strstr(p,"[") ) != NULL ) {
    if ( strstr(p, "[CDATA[") == p ) { // CDATA にいくつかの Trend Data がある
      p += 7; // "[CDATA[" の文字数

      for ( count=0 ; count<7 ; count++ ){
	q = strstr(p, ","); // カンマ区切り

	for (i = 0; p + i < q; i++) { // カンマ間の文字列を抽出 
	  buf[i] = *(p + i);
	}

	buf[i] = '\0';
	fprintf(fp,"%s\n", buf); // export to trand_database.txt

	p = p + i + 2; // 次のカンマを探索するための準備、スペースも含めてポインタを移動
	q = p; 
      }
    }
  }
  fclose(fp);
}
