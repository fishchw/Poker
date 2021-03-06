/***********************************************
 * File: PokerLogicManager.cpp
 *
 * Author: LYG
 * Date: ���� 2018
 *
 * Purpose:
 *
 * 
 **********************************************/
#include "PokerLogicManager.h"
#include "Math/Vector3.h"
#include "Scene.h"
#include "Timer.h"
#include "EffectManager.h"

PokerLogicManager::PokerLogicManager()
{
	mCardManager = new CardManager;
	mCardDealer = new RamdomDealer;
	Scene* S = new Scene;
	Scene::SetCurrentScene(S);
	Timer* T = new Timer;
	Timer::ThisInstance = T;
}

PokerLogicManager::~PokerLogicManager()
{
	Scene* S = Scene::GetCurrentScene();
	SAFE_DELETE(S);
	Scene::SetCurrentScene(nullptr);
	SAFE_DELETE(mCardManager);
	SAFE_DELETE(mCardDealer);
	SAFE_DELETE(Timer::ThisInstance);
	Timer::ThisInstance = nullptr;
}

bool PokerLogicManager::Initialise(HWND hWnd)
{
	Scene* S = Scene::GetCurrentScene();
	bool ret = S->Initialise(hWnd);
	if (!ret)
	{
		return false;
	}
	BuildPokerName();
	InitialiseAllResource();
	return true;
}

void PokerLogicManager::StartNewGame()
{
	int ret = MessageBoxA(NULL, "Are you sure to start a new game?", "Poker", MB_YESNO);
	if (ret == IDYES)
	{

	}
}

void PokerLogicManager::Update()
{
	Timer::ThisInstance->Update();
	Scene::GetCurrentScene()->Update();

	// Do a test
	float Effect_Interval = 10.0f;
	float EffectTimeLen = 6.0f;
	static unsigned int CurrentIndex = 0;
	static float CurrentInterval = 0.0f;
	static Effect* CurrentEffect = nullptr;
	CurrentInterval += float(Timer::GetInstance()->GetDelta()) * 0.001f;

	if (CurrentInterval >= Effect_Interval)
	{
		CurrentInterval -= Effect_Interval;
		EffectManager* EffectMgr = Scene::GetCurrentScene()->GetEffectManager();
		if (CurrentEffect)
		{
			EffectMgr->DestroyEffect(CurrentEffect->GetName());
			CurrentEffect = nullptr;
		}	
		SceneNode* BackGroundNode = Scene::GetCurrentScene()->GetBackGroundNode();
		Mesh* M = BackGroundNode->GetAttachMeshByIndex(0);
		float Acceleration = 0.5f;
		std::string NewTexturePath;
		if (CurrentIndex != Effect_Fade_Out)
		{
			NewTexturePath = Scene::GetCurrentScene()->GetResourceManager()->GetRandomBackGroundImagePath();
		}
		if (CurrentIndex == Effect_Norma_Brightness_Normal) Acceleration = 0.0f;
		if (CurrentIndex == Effect_Left_Right_Left) Acceleration = 2.0f;
		if (CurrentIndex == Effect_Rotate_Out_In) Acceleration = 4.0f;
		if (CurrentIndex == Effect_Separate_Tile)
		{
			Acceleration = -4.0f;
			EffectTimeLen = 9.0f;
		}
		else if (CurrentIndex == Effect_PerlinNoiseInOut)
		{
			Acceleration = 1.0f;
			EffectTimeLen = 9.99f;
		}
		else if(SimpleUScroll == CurrentIndex)
		{
			Acceleration = 1.0f;
			EffectTimeLen = 3.0f;
		}
		else
		{
			EffectTimeLen = 6.0f;
		}
		CurrentEffect = EffectMgr->CreateEffect("TestTest", (Effect_Type)CurrentIndex, EffectTimeLen, false, false, Acceleration, M, BackGroundNode, NewTexturePath.c_str());
		CurrentIndex++;
		CurrentIndex %= Effect_Max;
	}
}

void PokerLogicManager::BuildPokerName()
{
	char* szPoker[] = { "Diamond_", "CLub_", "Heart_", "Spade_" };
	for (int i = 0; i < Primary; i++)
	{
		for (int j = 0; j < Poker_Black_Joker; j++)
		{
			mPokerResourceName[i][j] = szPoker[i];
			char Buff[4] = { 0 };
			sprintf_s(Buff, 4, "%d", j);
			mPokerResourceName[i][j] += Buff;
		}
	}
	mBlackJokerName = "Black_Joker";
	mRedJokerName = "Red_Joker";
}

void PokerLogicManager::InitialiseAllResource()
{
	Vector3 vertex[4];
	float half_width = POKER_WIDTH / 2.0f;
	float half_height = POKER_HEIGHT / 2.0f;
	// left top
	vertex[0] = Vector3(-half_width, half_height, 0);
	// right top
	vertex[1] = Vector3(half_width, half_height, 0);
	// right bottom
	vertex[2] = Vector3(half_width, -half_height, 0);
	// left bottom
	vertex[3] = Vector3(-half_width, -half_height, 0);
	TextureManager* TextureMgr = Scene::GetCurrentScene()->GetTextureManager();
	MeshManager* MeshMgr = Scene::GetCurrentScene()->GetMeshManager();
	MaterialManager* MaterialMgr = Scene::GetCurrentScene()->GetMaterialManager();
	ResourceManager* ResourceMgr = Scene::GetCurrentScene()->GetResourceManager();
	RenderSystemD3D11* RS = Scene::GetCurrentScene()->GetRenderSystem();
	for (int i = 0; i < Primary; i++)
	{
		for (int j = 0; j < Poker_Black_Joker; j++)
		{
			// load textures
			std::string Path = ResourceMgr->GetPokerFullPath(PokerType(j), PokerClassify(i));
			D3d11Texture* Tex = TextureMgr->LoadTextureFromFile(mPokerResourceName[i][j], RS->GetD3d11Device(), Path.c_str(), false);
			// create materials
			Material* Mat = MaterialMgr->CreateMaterial(mPokerResourceName[i][j], SimpleTextureSample);
			Mat->SetTexture(Tex);
			// create meshes
			Mesh* M = MeshMgr->CreateQuad(mPokerResourceName[i][j], vertex);
			M->SetMaterial(Mat);

			// create all cards;
			Card* C = mCardManager->CreateCard(mPokerResourceName[i][j], PokerType(j), PokerClassify(i));
			// four pokers
			mCardDealer->PushCard(C);
			mCardDealer->PushCard(C);
			mCardDealer->PushCard(C);
			mCardDealer->PushCard(C);
		}
	}

	// create red and black joker
	std::string Path = ResourceMgr->GetPokerFullPath(Poker_Black_Joker, Primary);
	D3d11Texture* Tex = TextureMgr->LoadTextureFromFile(mBlackJokerName, RS->GetD3d11Device(), Path.c_str(), false);
	// create materials
	Material* Mat = MaterialMgr->CreateMaterial(mBlackJokerName, SimpleTextureSample);
	Mat->SetTexture(Tex);
	// create meshes
	Mesh* M = MeshMgr->CreateQuad(mBlackJokerName, vertex);
	M->SetMaterial(Mat);
	Card* C = mCardManager->CreateCard(mBlackJokerName, Poker_Black_Joker, Primary);
	mCardDealer->PushCard(C);
	mCardDealer->PushCard(C);
	mCardDealer->PushCard(C);
	mCardDealer->PushCard(C);

	Path = ResourceMgr->GetPokerFullPath(Poker_Red_Joker, Primary);
	Tex = TextureMgr->LoadTextureFromFile(mRedJokerName, RS->GetD3d11Device(), Path.c_str(), false);
	// create materials
	Mat = MaterialMgr->CreateMaterial(mRedJokerName, SimpleTextureSample);
	Mat->SetTexture(Tex);
	// create meshes
	M = MeshMgr->CreateQuad(mRedJokerName, vertex);
	M->SetMaterial(Mat);
	C = mCardManager->CreateCard(mRedJokerName, Poker_Red_Joker, Primary);
	mCardDealer->PushCard(C);
	mCardDealer->PushCard(C);
	mCardDealer->PushCard(C);
	mCardDealer->PushCard(C);
}

std::string PokerLogicManager::GetPokerName(PokerType PT, PokerClassify PC) const
{
	if (PT == Poker_Black_Joker)
	{
		return mBlackJokerName;
	}
	else if (PT == Poker_Red_Joker)
	{
		return mRedJokerName;
	}
	else
	{
		return mPokerResourceName[PC][PT];
	}
}


void PokerLogicManager::OnKeyDown(unsigned char Key)
{
	Camera* Cam = Scene::GetCurrentScene()->GetCurrentCamera();
	if (Key == 'A' || Key == 'a')
	{
		Cam->RollPitchYaw(0, -0.2, 0);
	}
	else if (Key == 'D' || Key == 'd')
	{
		Cam->RollPitchYaw(0, 0.2, 0);
	}
}

void PokerLogicManager::OnKeyUp(unsigned char Key)
{

}

void PokerLogicManager::OnLButtonDown(int x, int y, unsigned int wParam)
{

}

void PokerLogicManager::OnLButtonUp(int x, int y, unsigned int wParam)
{

}

void PokerLogicManager::OnRButtonDown(int x, int y, unsigned int wParam)
{

}

void PokerLogicManager::OnRButtonUp(int x, int y, unsigned int wParam)
{

}

void PokerLogicManager::OnLButtonDbclk(int x, int y, unsigned int wParam)
{

}

void PokerLogicManager::OnRButtonDbclk(int x, int y, unsigned int wParam)
{

}

void PokerLogicManager::OnMouseMove(int x, int y, unsigned int wParam)
{

}