#include "StdAfx.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <time.h>

int const HEART = 0;
int const SPADE = 1;
int const DIAMOND = 2;
int const CLUB = 3;
int const JOKER = 4;
int const ALL = 99;

int const DEF_SEQ_NO = 99;

int const BIN = -1;
int const TABLE = 0;
int const ME = 1;
int const COM1 = 2;
int const COM2 = 3;
int const COM3 = 4;
int const DUMMY_ID = -1;

int const CARD_MIN = 0;
int const CARD_MAX = 53;

int const END_GAME_CNT = 5;

int const ALL_PLAYER_NUM = 4;

int const DAIHINMIN = -10;
int const HINMIN = -5;
int const HEIMIN = 0;
int const FUGOU = 5;
int const DAIFUGOU = 10;
int const NOTHING = -1;

int const WEAKNESS = -100;

struct Card{
	int seq_num;
	int owner;
	int number;
	int mark;
	int strength;
	bool checked;
};

struct User{
	int id;
	char* name;
	int own_num;		// カード所持数
	bool end_flg;
	int score;
	int position;
};

struct Field{
	int Turn;
	int PassCnt;
	int PlayerNum;
	int DummyPlayerNum;		// 都落ちを考慮して格付けを行うために使用
	int TableCardNum;
	int GameCnt;		// 何回戦目かを記録
	int DaiFugouUser;
	int FugouUser;
	int HeiminUser;
	int HinminUser;
	int DaiHinminUser;

	bool Kakumei;
	bool Miyakoochi;
	bool ElevenBack;
	bool EightNagashi;
	bool Shibari;
	bool NumShibari;
};

struct Rule{
	bool Kakumei;
	bool Miyakoochi;
	bool ElevenBack;
	bool EightNagashi;
	bool Shibari;
	bool NumShibari;
};

struct Card cCard[CARD_MAX];
struct Card Dummy_Card = {DEF_SEQ_NO,DUMMY_ID,0,0,0,false};

struct User Me_User = {ME,"自分",0,false,0,HEIMIN};
struct User Com1_User = {COM1,"さとる",0,false,0,HEIMIN};
struct User Com2_User = {COM2,"ゆき",0,false,0,HEIMIN};
struct User Com3_User = {COM3,"けい",0,false,0,HEIMIN};
struct User Table_User = {TABLE,"テーブル",0,false,0,NOTHING};
struct User Dummy_User = {DUMMY_ID,"DUMMY",0,false,0,NOTHING};

struct Field Field = {DUMMY_ID,0,ALL_PLAYER_NUM,ALL_PLAYER_NUM,0,0,DUMMY_ID,DUMMY_ID,DUMMY_ID,DUMMY_ID,DUMMY_ID,false,false,false,false,false,false};

struct Rule Rule = {false,false,false,false,false,false};

//Card* cCard = new Card;

int main();

#pragma region "汎用メソッド"
int desc_user_cmp(const void *p1, const void *p2){
	struct User User1 = *(User *)p1;
	struct User User2 = *(User *)p2;

	int v1 = User1.score;
	int v2 = User2.score;

	if(v1 > v2){
		return -1;
	}
	else if(v1 == v2){
		return 0;
	}
	else{
		return 1;
	}

}

int scmp(const void *p1, const void *p2){
	char v1,v2;

	v1 = *(char *) p1;
	v2 = *(char *) p2;

	if(v1 < v2){
		return -1;
	}
	else if(v1 == v2){
		return 0;
	}
	else{
		return 1;
	}
}

int icmp(const void *p1, const void *p2){
	struct Card Card1 = *(Card *)p1;
	struct Card Card2 = *(Card *)p2;

	int v1 = Card1.strength;
	int v2 = Card2.strength;

	if(v1 < v2){
		return -1;
	}
	else if(v1 == v2){
		return 0;
	}
	else{
		return 1;
	}

}

int desc_icmp(const void *p1, const void *p2){
	struct Card Card1 = *(Card *)p1;
	struct Card Card2 = *(Card *)p2;

	int v1 = Card1.strength;
	int v2 = Card2.strength;

	if(v1 > v2){
		return -1;
	}
	else if(v1 == v2){
		return 0;
	}
	else{
		return 1;
	}

}

int get_user(struct User *wkUser, int viturn = Field.Turn){
	switch(viturn){
		case ME : *wkUser = Me_User; break;
		case COM1 : *wkUser = Com1_User; break;
		case COM2 : *wkUser = Com2_User; break;
		case COM3 : *wkUser = Com3_User; break;
		case TABLE : *wkUser = Table_User; break;
	}

	return 0;
}

int set_user(struct User *wkUser, int viturn = Field.Turn){
	switch(viturn){
		case ME : Me_User = *wkUser; break;
		case COM1 : Com1_User = *wkUser; break;
		case COM2 : Com2_User = *wkUser; break;
		case COM3 : Com3_User = *wkUser; break;
		case TABLE : Table_User = *wkUser; break;
	}

	return 0;
}

int get_next_turn(int now_turn = Field.Turn){
	struct User wkUser;
	
	now_turn++;
	if(now_turn > COM3){
		now_turn = ME;
	}

	get_user(&wkUser, now_turn);
	if(wkUser.end_flg == true){
		now_turn = get_next_turn(now_turn);
	}

	return now_turn;
}

int copy_fromto_card(struct Card* toCard,struct Card* fromCard){
	int i = 0;
	
	for(i = CARD_MIN; i < CARD_MAX; i++){
		toCard[i] = fromCard[i];
	}

	return 0;
}

int format_card(struct Card* rcCard){
	int i = 0;
	
	for(i = 0; i < sizeof(rcCard)/sizeof(rcCard[0]); i++){
		rcCard[i] = Dummy_Card;
	}

	return 0;
}

int clear_field_rule(){
	struct Card wkCard[CARD_MAX];
	int i = 0;

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	if(Field.ElevenBack == true){
		for(i = 0; i < CARD_MAX; i++){
			if(wkCard[i].mark != JOKER){
				wkCard[i].strength = wkCard[i].strength * -1;
			}
		}
	}

	// コミット
	copy_fromto_card(cCard, wkCard);

	Field.ElevenBack = false;
	Field.EightNagashi = false;
	Field.Shibari = false;
	Field.NumShibari = false;

	return 0;
}

int clear_rule(bool allflg = false){
	if(allflg == true){
		Rule.Kakumei = false;
		Rule.Miyakoochi = false;
		Rule.ElevenBack = false;
		Rule.EightNagashi = false;
		Rule.Shibari = false;
		Rule.NumShibari = false;
	}

	Field.Kakumei = false;
	Field.Miyakoochi = false;
	Field.ElevenBack = false;
	Field.EightNagashi = false;
	Field.Shibari = false;
	Field.NumShibari = false;

	return 0;
}

int clear_user(){
	int i = 0;
	struct User wkUser = Dummy_User;
	
	for(i = ME; i <= COM3; i++){
		get_user(&wkUser,i);
		wkUser.end_flg = false;
		wkUser.own_num = 0;
		set_user(&wkUser,i);
	}

	return 0;
}

int clear_checked(struct Card* rcCard){
	int i = 0;
	
	for(i = CARD_MIN; i < CARD_MAX; i++){
		rcCard[i].checked = false;
	}

	return 0;
}

int clear_seq_no(struct Card* rcCard){
	int i = 0;
	
	for(i = CARD_MIN; i < CARD_MAX; i++){
		rcCard[i].seq_num = DEF_SEQ_NO;
	}

	return 0;
}

int change_fromto_owner(struct Card* rcCard, int fromowner, int toowner){
	int i = 0;
	
	for(i = CARD_MIN; i < CARD_MAX; i++){
		if(rcCard[i].owner == fromowner){
			rcCard[i].owner = toowner;
		}
	}

	return 0;
}

bool get_card(struct Card* rcCard, int *pnum){
	int i = *pnum;

	for(;i < CARD_MAX; i++){
		if(rcCard[i].checked == true){
			*pnum = i;
			return true;
		}
	}

	return false;

}
int select_card(struct Card* vcCard, int *pcnt,int vimark = ALL, int vinumber = ALL, int viowner = ALL, int vistrengthFrom = ALL, int vistrengthTo = ALL, int visamenum = ALL){
	int i = 0;
	int icnt = 0;
	int same_cnt = 0;
	int joker_cnt = 0;
	bool isvalidate = false;
	struct Card wkCard[CARD_MAX];

	copy_fromto_card(wkCard, vcCard);

	for(i = CARD_MIN; i < CARD_MAX; i++){
		if(vimark == ALL || wkCard[i].mark == vimark){
			isvalidate = true;		
		}
		else{
			isvalidate = false;
		}

		if(isvalidate == true){
			if(vinumber == ALL || wkCard[i].number == vinumber){
				isvalidate = true;		
			}
			else{
				isvalidate = false;
			}
		}

		if(isvalidate == true){
			if(viowner == ALL || wkCard[i].owner == viowner){
				isvalidate = true;		
			}
			else{
				isvalidate = false;
			}
		}

		if(isvalidate == true){
			if(vistrengthFrom == ALL || wkCard[i].strength >= vistrengthFrom){
				isvalidate = true;		
			}
			else{
				isvalidate = false;
			}
		}

		if(isvalidate == true){
			if(vistrengthTo == ALL || wkCard[i].strength <= vistrengthTo){
				isvalidate = true;		
			}
			else{
				isvalidate = false;
			}
		}

		if(isvalidate == true){
			same_cnt = 0;
			joker_cnt = 0;
			if(visamenum == ALL || visamenum == 0){
				isvalidate = true;		
			}
			else{
				select_card(cCard, &same_cnt, ALL,  wkCard[i].number, wkCard[i].owner, ALL, ALL);
				select_card(cCard, &joker_cnt, JOKER, ALL, wkCard[i].owner);
				if((same_cnt+joker_cnt) >= visamenum){
					isvalidate = true;
				}
				else{
					isvalidate = false;
				}
			}
		}

		if(isvalidate == true){
			wkCard[i].checked = isvalidate;
			icnt++;
		}
	}

	copy_fromto_card(vcCard, wkCard);
	*pcnt = icnt;

	return 0;
}

int format_card(){
	int i = 0;
	int ii = 0;
	int iseq = 0;

	for(i = HEART; i <= CLUB; i++){
		for(ii = 1; ii <= 13; ii++){
			switch(i){
				case HEART: cCard[iseq].mark = HEART;break;
				case SPADE: cCard[iseq].mark = SPADE;break;
				case DIAMOND: cCard[iseq].mark = DIAMOND;break;
				case CLUB: cCard[iseq].mark = CLUB;break;
			}
			cCard[iseq].number = ii;
			cCard[iseq].owner = TABLE;
			cCard[iseq].checked = false;
			if(cCard[iseq].number <= 2){
				cCard[iseq].strength = cCard[iseq].number + 13;
			}
			else{
				cCard[iseq].strength = cCard[iseq].number;
			}
			iseq++;
		}
	}

	// ジョーカーの作成
	cCard[iseq].mark = JOKER;
	cCard[iseq].number = 99;
	cCard[iseq].owner = TABLE;
	cCard[iseq].strength = 99;
	cCard[iseq].checked = false;

	return 0;
}

#pragma endregion

#pragma region "ルール適用チェック"
bool chk_Kakumei(){
	int inum = 0;
	int icnt = 0;
	int i = 0;
	struct Card wkCard[CARD_MAX];
	bool bret = false;
	
	// ルール適用外は処理を抜ける
	if(Rule.Kakumei == false){
		return bret;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	clear_checked(wkCard);
	// TABLEのカードをチェック
	select_card(wkCard, &icnt,ALL,ALL,TABLE);

	// ４枚出ていたら革命
	if(icnt == 4){
		Field.Kakumei = true;
		for(i = 0; i < CARD_MAX; i++){
			if(wkCard[i].mark != JOKER){
				wkCard[i].strength = wkCard[i].strength * -1;
			}
		}

		bret = true;
	}

	// コミット
	copy_fromto_card(cCard, wkCard);

	return bret;
}

int chk_Miyakoochi(struct User vuUser){
	struct User wkUser = Dummy_User;
	struct Card wkCard[CARD_MAX];

	// ルール適用外、ルール適用後は処理を抜ける
	if(Rule.Miyakoochi == false || Field.Miyakoochi == true){
		return 0;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	// 前回大富豪の情報を取得
	get_user(&wkUser, Field.DaiFugouUser);

	if(Field.DaiFugouUser != DUMMY_ID && Field.DaiFugouUser != vuUser.id){
		// 大富豪が続けて大富豪になれなかったら大貧民に落ちて強制終了
		wkUser.score += DAIHINMIN;
		wkUser.position = DAIHINMIN;
		wkUser.own_num = 0;
		wkUser.end_flg = true;
		Field.DaiHinminUser = wkUser.id;
		Field.Miyakoochi = true;
		Field.PlayerNum--;
		
		// 対象ユーザのカードをBINに捨てる
		change_fromto_owner(wkCard, wkUser.id, BIN);
		
		// コミット
		set_user(&wkUser, wkUser.id);
		copy_fromto_card(cCard, wkCard);
	}

	return 0;
}

int chk_ElevenBack(){
	int inum = 0;
	int icnt = 0;
	int i = 0;
	struct Card wkCard[CARD_MAX];
	
	// ルール適用外、ルール適用後は処理を抜ける
	if(Rule.ElevenBack == false || Field.ElevenBack == true){
		return 0;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	clear_checked(wkCard);
	// TABLEのカードをチェック
	select_card(wkCard, &icnt, ALL, ALL, TABLE);
	get_card(wkCard, &inum);

	// 11だったら強さ逆転
	if(wkCard[inum].number == 11){
		Field.ElevenBack = true;
		for(i = 0; i < CARD_MAX; i++){
			if(wkCard[i].mark != JOKER){
				wkCard[i].strength = wkCard[i].strength * -1;
			}
		}
	}

	// コミット
	copy_fromto_card(cCard, wkCard);

	return 0;
}

bool chk_EightNagashi(struct User vuUser){
	int inum = 0;
	int icnt = 0;
	int i = 0;
	struct Card wkCard[CARD_MAX];
	
	// ルール適用外、ルール適用後は処理を抜ける
	if(Rule.EightNagashi == false || Field.EightNagashi == true){
		return false;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	clear_checked(wkCard);
	// TABLEのカードをチェック
	select_card(wkCard, &icnt, ALL, ALL, TABLE);
	get_card(wkCard, &inum);

	if(wkCard[inum].number == 8){
		// ターンは8を出したユーザ
		Field.Turn = vuUser.id;

		cout << "[8切り]" << endl;
		
		// コミットはしない
		return true;
	}

	// コミットはしない
	return false;
}

int chk_Bef_Shibari(int *iher_cnt, int *ispd_cnt, int *idia_cnt, int *iclb_cnt){
	int icnt = 0;
	struct Card wkCard[CARD_MAX];
	
	// ルール適用外のときは処理を抜ける
	if(Rule.Shibari == false){
		return 0;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	clear_checked(wkCard);
	// 変更前のTABLEのカードをチェック
	select_card(wkCard, &icnt, HEART, ALL, TABLE);
	*iher_cnt = icnt;

	clear_checked(wkCard);
	select_card(wkCard, &icnt, SPADE, ALL, TABLE);
	*ispd_cnt = icnt;

	clear_checked(wkCard);
	select_card(wkCard, &icnt, DIAMOND, ALL, TABLE);
	*idia_cnt = icnt;

	clear_checked(wkCard);
	select_card(wkCard, &icnt, CLUB, ALL, TABLE);
	*iclb_cnt = icnt;

	// コミットはしない
	return 0;
}

bool chk_Af_Shibari(struct Card *rcCard,int iher_cnt, int ispd_cnt, int idia_cnt, int iclb_cnt){
	int icnt = 0;
	int inum = 0;
	bool bret = true;
	int iaf_her_cnt;
	int iaf_spd_cnt;
	int iaf_dia_cnt;
	int iaf_clb_cnt;
	bool change_flg = false;

	struct Card wkCard[CARD_MAX];
	
	// ルール適用外のときは処理を抜ける
	if(Rule.Shibari == false){
		return true;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, rcCard);

	clear_checked(wkCard);
	// 変更後のTABLEのカードをチェック
	select_card(wkCard, &icnt, HEART, ALL, TABLE);
	iaf_her_cnt = icnt;

	clear_checked(wkCard);
	select_card(wkCard, &icnt, SPADE, ALL, TABLE);
	iaf_spd_cnt = icnt;

	clear_checked(wkCard);
	select_card(wkCard, &icnt, DIAMOND, ALL, TABLE);
	iaf_dia_cnt = icnt;

	clear_checked(wkCard);
	select_card(wkCard, &icnt, CLUB, ALL, TABLE);
	iaf_clb_cnt = icnt;

	// ジョーカーのマーク変換処理
	clear_checked(wkCard);
	select_card(wkCard, &icnt, JOKER, ALL, TABLE);
	get_card(wkCard, &inum);

	// しばりが適用済 かつ ジョーカーがテーブルに存在している
	if(Field.Shibari == true && icnt > 0){
		// 一枚不足しているマークをジョーカーで補う
		if(iher_cnt-iaf_her_cnt == 1){
			// メソッド呼び出し元の状態にワークを戻した後に更新する
			copy_fromto_card(wkCard, rcCard);
			wkCard[inum].mark = HEART;
			iaf_her_cnt++;
			change_flg = true;
		}

		if(ispd_cnt-iaf_spd_cnt == 1 && change_flg == false){
			// メソッド呼び出し元の状態にワークを戻した後に更新する
			copy_fromto_card(wkCard, rcCard);
			wkCard[inum].mark = SPADE;
			iaf_spd_cnt++;
			change_flg = true;
		}

		if(idia_cnt-iaf_dia_cnt == 1 && change_flg == false){
			// メソッド呼び出し元の状態にワークを戻した後に更新する
			copy_fromto_card(wkCard, rcCard);
			wkCard[inum].mark = DIAMOND;
			iaf_dia_cnt++;
			change_flg = true;
		}

		if(iclb_cnt-iaf_clb_cnt == 1 && change_flg == false){
			// メソッド呼び出し元の状態にワークを戻した後に更新する
			copy_fromto_card(wkCard, rcCard);
			wkCard[inum].mark = CLUB;
			iaf_clb_cnt++;
			change_flg = true;
		}

		// マーク変換処理が行われた場合は呼び出し元テーブルを更新する
		copy_fromto_card(rcCard, wkCard);
	}

	// 全てのマークの枚数が一致していれば縛り成立
	if(iher_cnt == iaf_her_cnt && ispd_cnt == iaf_spd_cnt
		&& idia_cnt == iaf_dia_cnt && iclb_cnt == iaf_clb_cnt){
			Field.Shibari = true;
			return true;
	}

	// 縛り不成立
	// 縛り適用前なら不成立でもOK
	if(Field.Shibari == false){
		return true;
	}
	else{
		return false;
	}
}

int chk_Bef_NumShibari(int *inum){
	int icnt = 0;
	int iseq = 0;
	bool bret = 0;
	struct Card wkCard[CARD_MAX];
	
	// ルール適用外のときは処理を抜ける
	if(Rule.NumShibari == false){
		return 0;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	clear_checked(wkCard);
	// 変更前のTABLEのカードをチェック
	select_card(wkCard, &icnt, ALL, ALL, TABLE);
	bret = get_card(wkCard, &iseq);
	if(bret == true){
		*inum = wkCard[iseq].strength;
	}
	else{
		*inum = NOTHING;
	}

	// コミットはしない
	return 0;
}

bool chk_Af_NumShibari(struct Card *rcCard,int ibef_num){
	int icnt = 0;
	int iaf_num = 0;
	int iseq = 0;
	int i = 0;
	struct Card wkCard[CARD_MAX];
	
	// ルール適用外のときは処理を抜ける
	if(Rule.NumShibari == false){
		return false;
	}

	// トランザクション開始
	copy_fromto_card(wkCard, rcCard);

	clear_checked(wkCard);
	// 変更後のTABLEのカードをチェック
	select_card(wkCard, &icnt, ALL, ALL, TABLE);
	get_card(wkCard, &iseq);
	iaf_num = wkCard[iseq].strength;

	// 数字が続いていれば、しばり成立
	if((iaf_num == ibef_num+1) || ((iaf_num == ibef_num-1))){
		Field.NumShibari = true;
		return true;
	}

	// 成立済で変更後がジョーカーなら成立とみなす
	if(Field.NumShibari == true && wkCard[iseq].mark == JOKER){
		return true;
	}

	// 縛り不成立
	// 縛り適用前なら不成立でもOK
	if(Field.NumShibari == false){
		return true;
	}
	else{
		return false;
	}
}

#pragma endregion

int distribute_card(){
	int i = 0;
	int itgt = 0;
	int iall = 0;
	int icnt = 0;
	int inum = 0;
	char* str;
	bool setflg = false;
	struct Card wkCard[CARD_MAX];
	struct User dfUser,fUser,hUser,dhUser;

	// 乱数系列の変更
    srand((unsigned) time(NULL));

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	for(iall = CARD_MIN; iall < CARD_MAX;){
		for(i = ME;i <= COM3; i++){
			itgt = CARD_MIN + (int)(rand()*((CARD_MAX-1)-CARD_MIN+1.0)/(1.0+RAND_MAX));
	
			while(setflg == false){
				if(itgt >= CARD_MAX){
					itgt = CARD_MIN;
				}

				if(wkCard[itgt].owner == TABLE){
					// 最後は可能な限りランダムで振る
					if(iall == CARD_MAX-1){
						if(Field.Turn != DUMMY_ID){
							i = get_next_turn();
						}
					}
					
					switch(i){
						case ME: wkCard[itgt].owner = ME; Me_User.own_num++; break;
						case COM1: wkCard[itgt].owner = COM1; Com1_User.own_num++; break;
						case COM2: wkCard[itgt].owner = COM2; Com2_User.own_num++; break;
						case COM3: wkCard[itgt].owner = COM3; Com3_User.own_num++; break;
					}
					setflg = true;
				}
				else{
					itgt++;
				}
			}

			setflg = false;
			iall++;
			if(iall >= CARD_MAX){
				break;
			}
		}
	}

	// カードの交換処理
	if(Field.GameCnt > 1){
		// ユーザ情報の取得
		get_user(&dfUser, Field.DaiFugouUser);
		get_user(&fUser, Field.FugouUser);
		get_user(&hUser, Field.HinminUser);
		get_user(&dhUser, Field.DaiHinminUser);


		// ①大貧民が大富豪に最も強いカード２枚を渡す
		qsort(wkCard, CARD_MAX, sizeof(Card), desc_icmp);
		clear_checked(wkCard);
		select_card(wkCard, &icnt, ALL, ALL, Field.DaiHinminUser);
		for(i = 0,inum = 0; i < 2; i++,inum++){
			get_card(wkCard, &inum);
			wkCard[inum].owner = Field.DaiFugouUser;
			switch(wkCard[inum].mark){
				case HEART: str = "ハート"; break;
				case SPADE: str = "スペード"; break;
				case DIAMOND: str = "ダイヤ"; break;
				case CLUB: str = "クラブ"; break;
				case JOKER: str = "ジョーカー"; break;
			}

			if(wkCard[inum].mark == JOKER){
				cout << dhUser.name << "大貧民" << "は、" << dfUser.name << "大富豪へ「ジョーカー」を献上しました。" << endl;
			}
			else{
				cout << dhUser.name << "大貧民" << "は、" << dfUser.name << "大富豪へ「" << str << "の" << wkCard[inum].number << "」を献上しました。" << endl;
			}
		}

		// ②大富豪が大貧民に最も弱いカード２枚を渡す
		qsort(wkCard, CARD_MAX, sizeof(Card), icmp);
		clear_checked(wkCard);
		select_card(wkCard, &icnt, ALL, ALL, Field.DaiFugouUser);
		for(i = 0,inum = 0; i < 2; i++,inum++){
			get_card(wkCard, &inum);
			wkCard[inum].owner = Field.DaiHinminUser;
			switch(wkCard[inum].mark){
				case HEART: str = "ハート"; break;
				case SPADE: str = "スペード"; break;
				case DIAMOND: str = "ダイヤ"; break;
				case CLUB: str = "クラブ"; break;
				case JOKER: str = "ジョーカー"; break;
			}

			if(wkCard[inum].mark == JOKER){
				cout << dfUser.name << "大富豪" << "は、" << dhUser.name << "大貧民へ「ジョーカー」を投げ捨てました。" << endl;
			}
			else{
				cout << dfUser.name << "大富豪" << "は、" << dhUser.name << "大貧民へ「" << str << "の" << wkCard[inum].number << "」を投げ捨てました。" << endl;
			}
		}

		// ③貧民が富豪へ最も強いカードを渡す
		inum = 0;
		qsort(wkCard, CARD_MAX, sizeof(Card), desc_icmp);
		clear_checked(wkCard);
		select_card(wkCard, &icnt, ALL, ALL, Field.HinminUser);
		get_card(wkCard, &inum);
		wkCard[inum].owner = Field.FugouUser;
		switch(wkCard[inum].mark){
			case HEART: str = "ハート"; break;
			case SPADE: str = "スペード"; break;
			case DIAMOND: str = "ダイヤ"; break;
			case CLUB: str = "クラブ"; break;
			case JOKER: str = "ジョーカー"; break;
		}

		if(wkCard[inum].mark == JOKER){
			cout << hUser.name << "貧民" << "は、" << fUser.name << "富豪へ「ジョーカー」を献上しました。" << endl;
		}
		else{
			cout << hUser.name << "貧民" << "は、" << fUser.name << "富豪へ「" << str << "の" << wkCard[inum].number << "」を献上しました。" << endl;
		}

		// ④富豪が貧民に最も弱いカードを渡す
		inum = 0;
		qsort(wkCard, CARD_MAX, sizeof(Card), icmp);
		clear_checked(wkCard);
		select_card(wkCard, &icnt, ALL, ALL, Field.FugouUser);
		get_card(wkCard, &inum);
		wkCard[inum].owner = Field.HinminUser;
		switch(wkCard[inum].mark){
			case HEART: str = "ハート"; break;
			case SPADE: str = "スペード"; break;
			case DIAMOND: str = "ダイヤ"; break;
			case CLUB: str = "クラブ"; break;
			case JOKER: str = "ジョーカー"; break;
		}

		if(wkCard[inum].mark == JOKER){
			cout << fUser.name << "富豪" << "は、" << hUser.name << "貧民へ「ジョーカー」を投げ捨てました。" << endl;
		}
		else{
			cout << fUser.name << "富豪" << "は、" << hUser.name << "貧民へ「" << str << "の" << wkCard[inum].number << "」を投げ捨てました。" << endl;
		}
	}

	// コミット
	copy_fromto_card(cCard, wkCard);

	return 0;
}

int have_valid_card(int viuser){
	int icnt = 0;
	int inum = 0;
	int itable_str = 0;
	struct Card wkCard[CARD_MAX];
	copy_fromto_card(wkCard, cCard);

	// テーブルのカードを取得
	clear_checked(wkCard);
	select_card(wkCard, &icnt,ALL,ALL,TABLE);
	get_card(wkCard, &inum);
	if(icnt > 0){
		itable_str = wkCard[inum].strength + 1;
	}
	else{
		itable_str = WEAKNESS;
	}
	// テーブルのカードより強いカードをチェック
	clear_checked(wkCard);
	select_card(wkCard, &icnt,ALL,ALL,viuser,itable_str,ALL,Field.TableCardNum);
	
	// ジョーカーは常に選択可能
	select_card(wkCard, &icnt,JOKER,ALL,viuser);

	copy_fromto_card(cCard, wkCard);

	return 0;
}

int disp(int viowner, bool isdisp){
	int i = 0;
	int j = 0;
	int ime = 0;
	int icnt = 0;
	struct User wkUser = Dummy_User;
	struct Card wkCard[CARD_MAX];

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);

	// シーケンス番号をクリア
	if(viowner == Field.Turn){
		clear_seq_no(wkCard);
	}

	for(i = CARD_MIN; i < CARD_MAX; i++){
		if(wkCard[i].owner == viowner){
			// 仮のシーケンス番号をナンバリング
			if(viowner == Field.Turn){
				wkCard[i].seq_num = ime;
			}

			// 自分のカード数をカウントアップ
			ime++;
		}
	}

	// ソート処理
	qsort(wkCard, CARD_MAX, sizeof(Card), icmp);

	// 正しいシーケンス番号をナンバリング
	if(viowner == Field.Turn){
		for(i = CARD_MIN; i < CARD_MAX; i++){
			if(wkCard[i].seq_num != DEF_SEQ_NO){
				wkCard[i].seq_num = j;
				j++;
			}
		}
	}

	if(viowner == ME && Field.Turn == ME){
		cout << " ａ  ｂ　ｃ　ｄ　ｅ　ｆ　ｇ　ｈ　ｉ　ｊ　ｋ　ｌ　ｍ　ｎ　　ｐ(パス)\n" << flush;
		cout << " " << flush;
		
		for(i = 0;i < CARD_MAX; i++){
			if(wkCard[i].checked == true){
				cout << "＊  " << flush;
			}
			else if(wkCard[i].owner == viowner){
				cout << "　  " << flush;
			}
		}

		cout << "\n" << flush;
	}

	cout << " " << flush;

	for(i = 0;i < ime; i++){
		cout << "__  " << flush;
	}

	cout << "\n" << flush;

	for(i = 0; i < CARD_MAX; i++){
		if(wkCard[i].owner == viowner){
			cout << "|" << flush;
			if(isdisp == true ){
				switch(wkCard[i].mark){
					/*case HEART: cout << "♥" << flush;break;
					case SPADE: cout << "♠" << flush;break;
					case DIAMOND: cout << "♦" << flush;break;
					case CLUB:cout << "♣" << flush;break;*/
					case HEART: cout << "Hr" << flush;break;
					case SPADE: cout << "Sp" << flush;break;
					case DIAMOND: cout << "Di" << flush;break;
					case CLUB:cout << "Cl" << flush;break;
					case JOKER:cout << "Jo" << flush;break;
				}
			}
			else{
				 cout << "？" << flush;
			}
			cout << "|" << flush;
		}
	}

	// ユーザ情報の取得
	get_user(&wkUser, viowner);

	cout << "【"<< wkUser.name <<"】" << flush;

	switch(wkUser.position){
		case DAIFUGOU : cout << "大富豪 " << wkUser.score << "pt\n" << flush;break;
		case FUGOU : cout << "富豪 " << wkUser.score << "pt\n" << flush;break;
		case HEIMIN : cout << "平民 " << wkUser.score << "pt\n" << flush;break;
		case HINMIN : cout << "貧民 " << wkUser.score << "pt\n" << flush;break;
		case DAIHINMIN : cout << "大貧民 " << wkUser.score << "pt\n" << flush;break;
		case NOTHING :
			cout << " " << Field.GameCnt << "回戦目" << flush;
			if(Field.Kakumei == true){
				cout << " [革命]" << flush;
			}
			if(Field.Miyakoochi == true){
				cout << " [都落ち]" << flush;
			}
			if(Field.ElevenBack == true){
				cout << " [11バック]" << flush;
			}
			if(Field.EightNagashi == true){
				cout << " [8切り]" << flush;
			}
			if(Field.Shibari == true){
				cout << " [マークしばり]" << flush;
			}
			if(Field.NumShibari == true){
				cout << " [数字しばり]" << flush;
			}
			cout << "\n" << flush;
			break;
	}

	for(i = 0; i < CARD_MAX; i++){
		if(wkCard[i].owner == viowner){
			cout << "|" << flush;
			if(isdisp == true ){
				if(wkCard[i].number != 10){
					cout << " " << flush;
				}

				if(wkCard[i].mark == JOKER){
					cout << " " << flush;
				}
				else{
					switch(wkCard[i].number){
						case 11 : cout << 'J' << flush; break;
						case 12 : cout << 'Q' << flush; break;
						case 13 : cout << 'K' << flush; break;
						case 1 : cout << 'A' << flush; break;
						default : cout << wkCard[i].number << flush; break;
					}
				}
			}
			else{
				cout << "??" << flush;
			}
			cout << "|" << flush;
		}
	}

	cout << "\n " << flush;

	for(i = 0;i < ime; i++){
		cout << "―  " << flush;
	}

	cout << "\n" << flush;

	// コミット
	copy_fromto_card(cCard, wkCard);

	return 0;
}

int disp_table(){
	int inum = 0;
	int icnt = 0;
	struct Card wkCard[CARD_MAX];
	copy_fromto_card(wkCard, cCard);

	clear_checked(wkCard);
	// テーブルのカード枚数を一度クリアする
	Field.TableCardNum = 0;

	// TABLEのカードをチェック
	select_card(wkCard, &icnt,ALL,ALL,TABLE);

	while(get_card(wkCard, &inum) == true){
		wkCard[inum].owner = TABLE;
		Field.TableCardNum++;
		inum ++;
	}

	copy_fromto_card(cCard, wkCard);
	cout << "\n\n" << flush;
	disp(TABLE, true);

	return 0;
}

int receive(){
	int i = 0;
	int j = 0;
	int iseq = 0;
	char sw;
	char line[100];
	int icnt = 0;
	bool setflg = false;
	int cur_num = 0;
	int iher_cnt = 0;
	int ispd_cnt = 0;
	int idia_cnt = 0;
	int iclb_cnt = 0;
	int iaf_num = 0;

	struct Card wkCard[CARD_MAX];
	struct User wkUser;

	// 文字配列の初期化
	for(i = 0; i < 100; i++){
		line[i] = 'z';
	}
	
	// 縛りの事前チェックを行う
	chk_Bef_Shibari(&iher_cnt, &ispd_cnt, &idia_cnt, &iclb_cnt);
	chk_Bef_NumShibari(&iaf_num);

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);
	wkUser = Me_User;

	// TABLEに置かれているカードをBINに捨てる
	change_fromto_owner(wkCard, TABLE, BIN);

	setflg = false;
	while(setflg == false){
		// 文字配列の初期化
		for(i = 0; i < 100; i++){
			line[i] = 'z';
		}
		i = 0;

		cin.getline(line, sizeof(line));

		sw = line[i];

		while(sw != '\0'){
			if(sw == 'a' || sw == 'b' || sw == 'c' || sw == 'd'
			   || sw == 'e' || sw == 'f' || sw == 'g' || sw == 'h'
			   || sw == 'i' || sw == 'j' || sw == 'k' || sw == 'l'
			   || sw == 'm' || sw == 'n' || sw == 'p'){
					setflg = true;
					i++;
					sw = line[i];
			}
			else{
				cout << "\nカードは、a～nのアルファベット、またはpを指定してください。" << endl;
				setflg = false;
				i = 0;
				break;
			}
		}

		// 入力文字をソートしておく（ジョーカー対策）
		qsort(line, 100, sizeof(char), scmp);
		icnt = i;

		i = 1;
		sw = line[i];

		while(setflg == true && i <= icnt){
			switch(sw){
				case 'a' : iseq = 0; break;
				case 'b' : iseq = 1; break;
				case 'c' : iseq = 2; break;
				case 'd' : iseq = 3; break;
				case 'e' : iseq = 4; break;
				case 'f' : iseq = 5; break;
				case 'g' : iseq = 6; break;
				case 'h' : iseq = 7; break;
				case 'i' : iseq = 8; break;
				case 'j' : iseq = 9; break;
				case 'k' : iseq = 10; break;
				case 'l' : iseq = 11; break;
				case 'm' : iseq = 12; break;
				case 'n' : iseq = 13; break;
				// パスの場合は更新処理を行わない
				case 'p' : Field.PassCnt++;return 0; break;
			}

			for(j = CARD_MIN; j < CARD_MAX; j++){
				if(wkCard[j].seq_num == iseq){
					if(wkCard[j].checked == true){
						// 指定可能なカードが見つかった
						wkCard[j].owner = TABLE;
						wkUser.own_num--;
						if(i == 1){
							cur_num = wkCard[j].number;
						}
						i++;
						sw = line[i];
						setflg = true;
						if((i == (sizeof(line)/sizeof(line[0]))) && (i == Field.TableCardNum || Field.TableCardNum == 0)){
							break;
						}
						else if(cur_num != wkCard[j].number && wkCard[j].mark != JOKER){
							cout << "\nカードを複数枚出す場合は、全て同じ数字にしてください。" << endl;
							// ロールバック
							copy_fromto_card(wkCard, cCard);
							change_fromto_owner(wkCard, TABLE, BIN);
							wkUser = Me_User;

							setflg = false;
							i = 0;
							cur_num = 0;
							break;
						}
					}
					else{
						cout << "\n＊がついているカード以外は選択できません。" << endl;
						// ロールバック
						copy_fromto_card(wkCard, cCard);
						change_fromto_owner(wkCard, TABLE, BIN);
						wkUser = Me_User;

						setflg = false;
						i = 0;
						cur_num = 0;
						break;
					}
				}
				else if(setflg == false && j == (CARD_MAX-1)){
					cout << "\n指定したアルファベットに対応するカードがありません。" << endl;
					// ロールバック
					copy_fromto_card(wkCard, cCard);
					change_fromto_owner(wkCard, TABLE, BIN);
					wkUser = Me_User;

					setflg = false;
					i = 0;
					cur_num = 0;
					break;
				}
			}
		}

		if(setflg == true && Field.TableCardNum > 0 && (i-1) != Field.TableCardNum){
			cout << "\nカードの枚数が正しくありません。" << endl;
			// ロールバック
			copy_fromto_card(wkCard, cCard);
			change_fromto_owner(wkCard, TABLE, BIN);
			wkUser = Me_User;

			setflg = false;
			i = 0;
			cur_num = 0;
		}

		// 数字縛りの事後チェックを行う
		if(setflg == true && chk_Af_NumShibari(wkCard,iaf_num) == false){
			cout << "\n数字しばりが発生しているため、数字を階段状にする必要があります。" << endl;
			// ロールバック
			copy_fromto_card(wkCard, cCard);
			change_fromto_owner(wkCard, TABLE, BIN);
			wkUser = Me_User;

			setflg = false;
			i = 0;
			cur_num = 0;
		}

		// 縛りの事後チェックを行う
		if(setflg == true && chk_Af_Shibari(wkCard, iher_cnt, ispd_cnt, idia_cnt, iclb_cnt) == false){
			cout << "\nしばりが発生しているため、マークを合わせる必要があります。" << endl;
			// ロールバック
			copy_fromto_card(wkCard, cCard);
			change_fromto_owner(wkCard, TABLE, BIN);
			wkUser = Me_User;

			setflg = false;
			i = 0;
			cur_num = 0;
		}
	}

	// コミット
	copy_fromto_card(cCard, wkCard);
	Me_User = wkUser;
	Field.PassCnt = 0;

	return 0;
}

int auto_choice(){
	int i = 0;
	int j = 0;
	int iseq = 0;
	bool setflg = false;
	char dummy;
	int chkcnt = 0;
	int cur_num = 0;
	int iher_cnt = 0;
	int ispd_cnt = 0;
	int idia_cnt = 0;
	int iclb_cnt = 0;
	int ifst_j = 0;
	int iaf_num = 0;

	struct Card wkCard[CARD_MAX];
	struct User wkUser;

	// 縛りの事前チェックを行う
	chk_Bef_Shibari(&iher_cnt, &ispd_cnt, &idia_cnt, &iclb_cnt);
	chk_Bef_NumShibari(&iaf_num);

	// トランザクション開始
	copy_fromto_card(wkCard, cCard);
	get_user(&wkUser);

	// TABLEに置かれているカードをBINに捨てる
	change_fromto_owner(wkCard, TABLE, BIN);

	for(j = CARD_MIN; j < CARD_MAX; j++){
		if(wkCard[j].seq_num != DEF_SEQ_NO){
			if(wkCard[j].checked == true && (chkcnt == 0 || wkCard[j].number == cur_num || wkCard[j].mark == JOKER)){
				// 指定可能なカードが見つかった
				wkCard[j].owner = TABLE;
				wkUser.own_num--;
				chkcnt++;
				if(chkcnt == 1){
					cur_num = wkCard[j].number;
					ifst_j = j;
				}

				if(Field.TableCardNum > 0){
					if(chkcnt == Field.TableCardNum){
						// 縛りの事後チェックを行う
						if(chk_Af_Shibari(wkCard, iher_cnt, ispd_cnt, idia_cnt, iclb_cnt) == true
							&& chk_Af_NumShibari(wkCard, iaf_num) == true){
							setflg = true;
							break;
						}
						else{
							// ロールバックし、違う組み合わせを見つける
							copy_fromto_card(wkCard, cCard);
							change_fromto_owner(wkCard, TABLE, BIN);
							get_user(&wkUser);
							chkcnt = 0;
							j = ifst_j;
						}
					}
				}
				else{
						setflg = true;
				}
			}
		}
	}

	if(setflg == false){
		Field.PassCnt++;
		cout << "【" << wkUser.name << "】はパスをしました。" << endl;
		dummy = getche();
		// パスの場合は更新を行わない
		return 0;
	}

	// コミット
	copy_fromto_card(cCard, wkCard);
	set_user(&wkUser);
	Field.PassCnt = 0;
	dummy = getche();

	return 0;
}

int do_disp(){
	struct User wkUser;
	bool bclear = false;

	// 現在のターンのユーザ情報をワークにコピー
	get_user(&wkUser);

	// 他プレイヤーが全員パスなら場を流す
	if(Field.PassCnt == (Field.PlayerNum-1)){
		change_fromto_owner(cCard, TABLE, BIN);
		Field.PassCnt = 0;
		// 場に適用されていた１回限りのルールを破棄する
		clear_field_rule();
	}

	// 画面表示を行う
	disp_table();

	// 選択可能なカードを特定する
	have_valid_card(Field.Turn);
	disp(ME, true);
	disp(COM1, false);
	disp(COM2, false);
	disp(COM3, false);

	// ゲーム終了の判断
	if(Field.PlayerNum <= 1){
		cout << "ゲーム終了です。" << endl;
		if(Field.Miyakoochi == true){
			// 都落ちが適用済の場合は最下位でも貧民
			wkUser.score += HINMIN;
			wkUser.position = HINMIN;
			Field.HinminUser = wkUser.id;
		}
		else{
			wkUser.score += DAIHINMIN;
			wkUser.position = DAIHINMIN;
			Field.DaiHinminUser = wkUser.id;
		}
		set_user(&wkUser);
		
		// メインの再帰呼び出し
		main();
	}

	switch(Field.Turn){
		case ME:  cout << "【" << Me_User.name << "】の番です。" << endl;break;
		case COM1:  cout << "【" << Com1_User.name << "】の番です。" << endl;break;
		case COM2:  cout << "【" << Com2_User.name << "】の番です。" << endl;break;
		case COM3:  cout << "【" << Com3_User.name << "】の番です。" << endl;break;
	}

	if(Field.Turn == ME){
		receive();
	}
	else{
		auto_choice();
	}

	// 選択後のユーザ情報をワークにセット
	get_user(&wkUser);

	// 革命が行われたかをチェックする
	//bclear = chk_Kakumei();
	if(Field.PassCnt == 0){
		chk_Kakumei();
	}

	// 11バックが行われたかをチェックする
	chk_ElevenBack();

	// 上がれたかを判断する
	if(wkUser.own_num <= 0){
		switch(Field.DummyPlayerNum){
			case ALL_PLAYER_NUM :
					// 都落ちをチェックする
					chk_Miyakoochi(wkUser);
					wkUser.score += DAIFUGOU;
					wkUser.position = DAIFUGOU;
					Field.DaiFugouUser = wkUser.id;
					break;
			case ALL_PLAYER_NUM-1 :
					wkUser.score += FUGOU;
					wkUser.position = FUGOU;
					Field.FugouUser = wkUser.id;
					break;
			case ALL_PLAYER_NUM-2 :
					wkUser.score += HINMIN;
					wkUser.position = HINMIN;
					Field.HinminUser = wkUser.id;
					break;
			case ALL_PLAYER_NUM-3 :
					wkUser.score += DAIHINMIN;
					wkUser.position = DAIHINMIN;
					Field.DaiHinminUser = wkUser.id;
					break;
		}
		wkUser.end_flg = true;
		set_user(&wkUser);
		Field.PlayerNum--;
		Field.DummyPlayerNum--;
	}

	// ターンの変更
	Field.Turn = get_next_turn();

	// 8流しが行われたかをチェックする
	if(bclear == false){
		bclear = chk_EightNagashi(wkUser);
	}

	// 場を強制的に流していいか判断
	if(bclear == true){
		Field.PassCnt = (Field.PlayerNum-1);
	}

	// 再帰処理
	do_disp();

	return 0;
}

int select_rule(){
	int i = 0;
	int j = 0;
	int iseq = 0;
	char sw;
	char line[100];
	int icnt = 0;
	bool setflg = false;
	int cur_num = 0;

	struct Card wkCard[CARD_MAX];
	struct User wkUser;

	// 文字配列の初期化
	for(i = 0; i < 100; i++){
		line[i] = 'z';
	}

	setflg = false;
	while(setflg == false){
		// ルールのクリア
		clear_rule(true);

		cout << "適用するルールのアルファベットを指定してください。" << endl;
		cout << "a：革命" << endl;
		cout << "b：都落ち" << endl;
		cout << "c：11バック" << endl;
		cout << "d：8切り" << endl;
		cout << "e：マークしばり" << endl;
		cout << "f：数字しばり" << endl;
		cout << "x：何も適用しない" << endl;

		cin.getline(line, sizeof(line));

		i = 0;
		sw = line[i];

		while(sw != '\0'){
			if(sw == 'a' || sw == 'b' || sw == 'c' || sw == 'd'
			   || sw == 'e' || sw == 'f' || sw == 'x'){
					setflg = true;
					i++;
					sw = line[i];
			}
			else{
				cout << "\nカードは、a～fのアルファベット、またはxを指定してください。" << endl;
				setflg = false;
				i = 0;
				break;
			}
		}
	}

	i = 0;
	sw = line[i];

	while(sw != '\0'){
		if(sw == 'x'){
			clear_rule(true);
			break;
		}
		else{
			switch(sw){
				case 'a' : Rule.Kakumei = true; break;
				case 'b' : Rule.Miyakoochi = true; break;
				case 'c' : Rule.ElevenBack = true; break;
				case 'd' : Rule.EightNagashi = true; break;
				case 'e' : Rule.Shibari = true; break;
				case 'f' : Rule.NumShibari = true; break;
			}
			i++;
			sw = line[i];
		}
	}

	if(Rule.Kakumei == true){
		cout << "<革命>" << endl;
	}

	if(Rule.Miyakoochi == true){
		cout << "<都落ち>" << endl;
	}

	if(Rule.ElevenBack == true){
		cout << "<11バック>" << endl;
	}

	if(Rule.EightNagashi == true){
		cout << "<8切り>" << endl;
	}

	if(Rule.Shibari == true){
		cout << "<マークしばり>" << endl;
	}

	if(Rule.NumShibari == true){
		cout << "<数字しばり>" << endl;
	}

	return 0;
}

int clear_status(){
	Field.DaiFugouUser = DUMMY_ID;
	Field.FugouUser = DUMMY_ID;
	Field.HeiminUser = DUMMY_ID;
	Field.HinminUser = DUMMY_ID;
	Field.DaiHinminUser = DUMMY_ID;

	Field.GameCnt = 0;
	Field.PassCnt = 0;
	Field.PlayerNum = ALL_PLAYER_NUM;
	Field.DummyPlayerNum = ALL_PLAYER_NUM;

	Me_User.position = HEIMIN;
	Com1_User.position = HEIMIN;
	Com2_User.position = HEIMIN;
	Com3_User.position = HEIMIN;

	Me_User.score = 0;
	Com1_User.score = 0;
	Com2_User.score = 0;
	Com3_User.score = 0;

	return 0;
}

int main(){
	struct Card wkCard[CARD_MAX];
	int icnt = 0;
	int inum = 0;
	char dummy_line[100];
	struct User Users[ALL_PLAYER_NUM];

	// ゲームカウントのアップ
	Field.GameCnt++;

	// ルールの適用設定
	if(Field.GameCnt == 1){
		select_rule();
	}
	else{
		// 場のルール適用状況をクリアする
		clear_rule();
	}

	// 5回戦が終わったら終了
	if(Field.GameCnt > 5){
		cout << "全ての対戦が終了しました。" << endl;

		// 結果の集計
		Users[0] = Me_User;
		Users[1] = Com1_User;
		Users[2] = Com2_User;
		Users[3] = Com3_User;

		qsort(Users, ALL_PLAYER_NUM, sizeof(User), desc_user_cmp);
		cout << "１位：" << Users[0].name << endl;
		cout << "２位：" << Users[1].name << endl;
		cout << "３位：" << Users[2].name << endl;
		cout << "４位：" << Users[3].name << endl;
		cout << "\n\n\n\n" << endl;

		cin.getline(dummy_line, sizeof(dummy_line));
		clear_status;
		main();
	}

	// ユーザ情報の初期化
	clear_user();

	// 場に残っている人数の初期化
	Field.PlayerNum = ALL_PLAYER_NUM;
	Field.DummyPlayerNum = ALL_PLAYER_NUM;

	// カードの初期化
	format_card();

	// 札を配る
	distribute_card();

	// ワークテーブルに表示テーブルの情報をコピー
	copy_fromto_card(wkCard, cCard);

	// 初回はダイヤの３を持っている人から始める
	if(Field.GameCnt == 1){
		clear_checked(wkCard);
		select_card(wkCard, &icnt,DIAMOND,3);
		get_card(wkCard, &inum);
		Field.Turn = wkCard[inum].owner;
	}
	else{
		// 2回目以降は大貧民から始める
		Field.Turn = Field.DaiHinminUser;
	}

	// 表示処理
	do_disp();

	return 0;
}
