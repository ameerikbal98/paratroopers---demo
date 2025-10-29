#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

	

//simple physics engine with a bit of flexibility
struct cPhysics
{
	float fpx = 0.0f,fpy = 0.0f;
	float fvx = 0.0f,fvy = 0.0f;
	float fAngle = 0.0f;
	float fVelMag = 1.0f;
	float fAcc = 0.0f;
	char cType = 'n';
	bool bDead = false;
	float fDeploy = 0.0f;
	bool bIsDeploy = false;
	float fTotalDistanceTravelledX = 0.0f;
	cPhysics(float x,float y, float angle, float velMag, char c)
	{
		fpx = x;
		fpy = y;
		fAngle = angle;
		fVelMag = velMag;
		fvx = fVelMag * cosf(angle);
		fvy = fVelMag * sinf(angle);
		cType = c;
		bDead = false;
	}

	void UpdateCoordinates(float fTime)
	{
		float oldx = fpx;
		fpx += fvx * fTime;
		fpy += fvy * fTime;
		fTotalDistanceTravelledX += fabs(oldx - fpx);
	}
};





class Paratroopers : public olc::PixelGameEngine
{

public:
	Paratroopers()
	{
		sAppName = "Example";
	}
	
private:
	//Stores the player part of the screen
	int nSWidth = 0;
	int nSHeight = 0;

	//Canon position in screen
	olc::vi2d viBaseRectPos;

	std::list<std::unique_ptr<cPhysics>> vecAerialBullets;
	std::list<std::unique_ptr<cPhysics>> vecAerialHelicopter;
	olc::vf2d bulletShape[10];

	//Sprites
	olc::Sprite *sprCanon;
	olc::Sprite *sprBase;
	olc::Sprite *sprBullet;
	olc::Sprite *sprHelicopter1;
	olc::Sprite *sprHelicopter2;
	olc::Decal *decCanon;
	olc::Decal *decBase;

	//canon variables
	float fCanonAngle = 0.0f;
	olc::vf2d vfCanonPos;
	olc::vf2d vfBasePos;

	//Score variables
	int nScore = 0;
	int nHighScore = 0;

	int nMaxFlyingObjects = 2;


	//Enemy variables

	int nMaxEnemy = 6;
	int nCurrentEnemy = 0;
	float fTimePassed = 0.0f;


public:
	bool OnUserCreate() override
	{
		std::srand(std::time({})); 
		// Called once at the start, so create things here
		nSWidth = ScreenWidth();
		nSHeight = ScreenHeight() - 30;
		float fCanonBasePosX = (float)nSWidth / 2.0f - 15.0f;
		float fCanonBasePosY = (float)nSHeight - 30.0f;

		viBaseRectPos = { (int)fCanonBasePosX , (int)fCanonBasePosY };
		vfCanonPos = {(float)nSWidth / 2.0f - 1.0f,(float)nSHeight - 30.0f - 6.0f};
		vfBasePos = {(float)nSWidth / 2 - 8,(float)nSHeight - 30 - 16};


		for(int i = 0; i < 10; i++)
		{
			bulletShape[i] = { 2.0f * cosf(3.141592f / ((float)i + 1.0f)), 2.0f * sinf(3.141592f / ((float)i + 1.0f))};
		}

		sprCanon = new olc::Sprite("./res/canon.png");
		sprBase = new olc::Sprite("./res/base.png");
		sprBullet = new olc::Sprite("./res/bullet.png");
		sprHelicopter1 = new olc::Sprite("./res/helicopter.png");
		sprHelicopter2 = new olc::Sprite("./res/helicoptermirror.png");
		decCanon = new olc::Decal(sprCanon);
		decBase = new olc::Decal(sprBase);


		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		FillRect(0,0,ScreenWidth(),ScreenHeight(),olc::BLACK);


		//Get User Input
		if(GetKey(olc::Key::LEFT).bHeld)
		{
			fCanonAngle -= 1.0f * fElapsedTime;
		}
		if(GetKey(olc::Key::RIGHT).bHeld)
		{
			fCanonAngle += 1.0f * fElapsedTime;
		}
		if(GetKey(olc::Key::SPACE).bReleased)
		{
			vecAerialBullets.push_back(std::make_unique<cPhysics>(vfCanonPos.x + 16 * cosf(fCanonAngle - 3.14592 / 2.0f), vfCanonPos.y + 16 * sinf(fCanonAngle - 3.141592 / 2.0f),fCanonAngle - 3.141592 / 2.0f,100.0f,'b'));
		}

		if(GetMouse(0).bReleased)
		{
			std::cout << "Mouse: " << GetMouseX() << " " << GetMouseY() << std::endl;
			std::cout << vfCanonPos.x + 16 * cosf(fCanonAngle - 3.14592 / 2.0f) << " " << vfCanonPos.y + 16 * sinf(fCanonAngle - 3.141592 / 2.0f) << std::endl;
			std::cout << fCanonAngle << std::endl;
		}
		fTimePassed += fElapsedTime;

		if(nCurrentEnemy == 0 && (int)fTimePassed % 3 == 0) // when enemy on the vector is zero generate a random wave of enemies
		{
			int nMaxEnemyCurrent = rand() % nMaxEnemy;
			for(int i = 0; i < nMaxEnemyCurrent; i++)
			{
				int dir = rand() % 2;
				float offsetx = 16;
				float offsety = 10;
				if(dir == 0)
				{
					float posx = 0 - i * offsetx;
					float posy = 3 + rand() % 5 + offsety * i;
					cPhysics *phy = new cPhysics(posx, posy, 0.0f, 40,'h');
					phy->fDeploy = rand() % ScreenWidth();
					if(phy->fDeploy > (ScreenWidth() / 2 - 15.0f) && phy->fDeploy < (ScreenWidth() / 2 + 15.0f))
					{
						phy->fDeploy > (ScreenWidth() / 2) ? phy->fDeploy + 15.0f : phy->fDeploy - 15.0f;
					}
					vecAerialHelicopter.push_back(std::unique_ptr<cPhysics>(phy));
					nCurrentEnemy ++;
				}
				else
				{
					float posx = ScreenWidth() + offsetx;
					float posy = 3 + rand() % 5 + offsety * i;
					cPhysics *phy = new cPhysics(posx, posy, 3.141592f, 40,'h');
					phy->fDeploy = rand() % ScreenWidth();
					if(phy->fDeploy > (ScreenWidth() / 2 - 15.0f) && phy->fDeploy < (ScreenWidth() / 2 + 15.0f))
					{
						phy->fDeploy > (ScreenWidth() / 2) ? phy->fDeploy + 15.0f : phy->fDeploy - 15.0f;
					}
					vecAerialHelicopter.push_back(std::unique_ptr<cPhysics>(phy));
					nCurrentEnemy ++;
				}
			} 
		}

		if(fCanonAngle < -3.141592f / 2.0f)
		{
			fCanonAngle = -3.141592 / 2.0f;
		}
		if(fCanonAngle > 3.141592f / 2.0f)
		{
			fCanonAngle = 3.141592 / 2.0f;
		}

		
		for(auto &f : vecAerialBullets) 
		{
			f->UpdateCoordinates(fElapsedTime);
			if(f->fpx < 0 || f->fpy < 0 || f->fpx >= ScreenWidth() || f->fpy >= ScreenHeight())
				f->bDead = true;


			for( auto &h : vecAerialHelicopter) //collision detection
			{
				if(f->fpx > h->fpx && f->fpx < (h->fpx + 16) && f->fpy < (h->fpy + 8)) //checs the top left position of bullet with the helicopter
				{
					f->bDead = true;
					h->bDead = true;
				}
				if((f->fpx + 2) > h->fpx && (f->fpx + 2) < (h->fpx + 16) && f->fpy < (h->fpy + 8)) // checks the top right postion with the helicopter
				{
					f->bDead = true;
					h->bDead = true;
				}
			}
			DrawSprite(f->fpx,f->fpy,sprBullet);
		}
		SetPixelMode(olc::Pixel::MASK);
		for( auto &f: vecAerialHelicopter)
		{
			f->UpdateCoordinates(fElapsedTime);
			// if(f->fpy < 0 || f->fpx >= ScreenWidth() || f->fpy >= ScreenHeight())
			// 	f->bDead = true; 
			if(f->fTotalDistanceTravelledX > ScreenWidth() + 40)
				f->bDead = true;
			if(f->fAngle == 0)
				DrawSprite(f->fpx,f->fpy,sprHelicopter1);
			if(f->fAngle == 3.141592f)
				DrawSprite(f->fpx,f->fpy,sprHelicopter2);
		}
		SetPixelMode(olc::Pixel::NORMAL);

		//collision detection
		

		vecAerialBullets.remove_if([](std::unique_ptr<cPhysics> &o) {return o->bDead;});
		vecAerialHelicopter.remove_if([](std::unique_ptr<cPhysics> &o) {return o->bDead;});
		nCurrentEnemy = vecAerialHelicopter.size();
		if(nCurrentEnemy == 0 && fTimePassed > 3.0f)
		{
			fTimePassed = 1.0f;
		}
		
		//DrawBorders
		DrawLine({0,nSHeight + 1}, { nSWidth, nSHeight + 1 }, olc::BLUE);
		// called once per frame
		FillRect(viBaseRectPos, {30 , 30}, olc::GREY);

		//Draw score
		DrawString({10,nSHeight + 3},"Score: ");
		DrawString({55,nSHeight + 3},std::to_string(nScore));

		DrawDecal(vfBasePos,decBase);
		DrawRotatedDecal(vfCanonPos,decCanon,fCanonAngle,{0.0f,15.0f},{1.0f,1.0f},olc::CYAN);

		return true;
	}

	bool OnUserDestroy() override
	{
		return true;
	}
	
};


int main()
{
	Paratroopers game;
	if (game.Construct(256, 228, 4, 4))
		game.Start();

	return 0;
}
