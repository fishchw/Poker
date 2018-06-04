/*!
 * \file PokerDef.h
 *
 * \author LYG
 * \date ���� 2018
 *
 * 
 */
#pragma once

#define WINDOW_WIDTH		1600
#define WINDOW_HEIGHT		1080
// Four pokers
#define POKER_COUNT			4
#define POKER_PIXEL_WIDTH	105
#define POKER_PIXEL_HEIGHT	150
#define POKER_SHOW_PIXEL	20

#define POKER_WIDTH			float(POKER_PIXEL_WIDTH) / float(WINDOW_WIDTH)
#define POKER_HEIGHT		POKER_WIDTH * float(POKER_PIXEL_HEIGHT) / float(POKER_PIXEL_WIDTH)
#define POKER_DEPTH			0.001
// ���ͣ��������ƵĴ�С��ϵ����С����
enum PokerType
{
	Poker_2,
	Poker_3,
	Poker_4,
	Poker_5,
	Poker_6,
	Poker_7,
	Poker_8,
	Poker_9,
	Poker_10,
	Poker_11,
	Poker_12,
	Poker_13,
	Poker_1,
	Poker_Black_Joker,
	Poker_Red_Joker,
	Poker_Type_Max,
};

// ���ֻ�ɫ
enum PokerClassify
{
	Diamond,
	CLub,
	Heart,
	Spade,
	Primary,			// Joker must in this group, others not.
	PC_Max,
};