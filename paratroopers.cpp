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
	bool bStable = false;
	bool bParachute = false;
	bool bPrachuteDestroyed = false;
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
	std::list<std::unique_ptr<cPhysics>> vecAerialTroopers;
	olc::vf2d bulletShape[10];

	//Sprites
	olc::Sprite *sprCanon;
	olc::Sprite *sprBase;
	olc::Sprite *sprBullet;
	olc::Sprite *sprHelicopter1;
	olc::Sprite *sprHelicopter2;
	olc::Sprite *sprTroopers;
	olc::Sprite *sprParachute;
	olc::Decal *decCanon;
	olc::Decal *decBase;

	//canon variables
	float fCanonAngle = 0.0f;
	olc::vf2d vfCanonPos;
	olc::vf2d vfBasePos;
	float fBulletSpeed = 300.0f;

	//Score variables
	int nScore = 0;
	int nHighScore = 0;


	//Enemy variables

	int nMaxEnemy = 6;
	int nCurrentEnemy = 0;
	float fTimePassed = 0.0f;
	int nEnemyLandedLeft = 0;
	int nEnemyLandedRight = 0;
	float nHelicopterSpeed = 40.0f;

	//Game over variable
	bool bGameOverLeft = false;
	bool bGameOverRight = false;

	bool bGameStart = false;



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
		sprTroopers = new olc::Sprite("./res/trooper.png");
		sprParachute = new olc::Sprite("./res/parachute.png");
		decCanon = new olc::Decal(sprCanon);
		decBase = new olc::Decal(sprBase);


		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if(!bGameStart)
		{
			FillRect(0,0,ScreenWidth(),ScreenHeight(),olc::BLACK);
			DrawString({ 0,((float)ScreenHeight() / 2.0f) - 32.0f},"GAME Will Finish if more than 4");
			DrawString({ 0,((float)ScreenHeight() / 2.0f) - 24.0f},"paratroopers land on any side of");
			DrawString({ 0,((float)ScreenHeight() / 2.0f) - 16.0f},"the canon. Control's are Left ");
			DrawString({ 0,((float)ScreenHeight() / 2.0f) - 8.0f},"and Right Key to rotate the");
			DrawString({ 0,((float)ScreenHeight() / 2.0f)},"canon and space Bar to Fire. Pre");
			DrawString({ 0,((float)ScreenHeight() / 2.0f) + 8.0f},"ss Space Bar to start the game");
			if(GetKey(olc::Key::SPACE).bReleased)
			{
				bGameStart = true;
				bGameOverLeft = false;
				bGameOverRight = false;
				vecAerialBullets.clear();
				vecAerialHelicopter.clear();
				vecAerialTroopers.clear();
				nScore = 0;
				nEnemyLandedLeft = 0;
				nEnemyLandedRight = 0;
				nHelicopterSpeed = 40.0f;
			}

			return true;
		}


		if(bGameOverLeft || bGameOverRight)
		{
			DrawString({((float)ScreenWidth() / 2.0f) - 120.0f,((float)ScreenHeight() / 2.0f) - 8.0f},"GAME OVER : Press Space");
			if(GetKey(olc::Key::SPACE).bReleased)
				bGameStart = false;
		}
		else
		{
		FillRect(0,0,ScreenWidth(),ScreenHeight(),olc::BLACK);


		//Get User Input
		if(GetKey(olc::Key::LEFT).bHeld)
		{
			fCanonAngle -= 2.0f * fElapsedTime;
		}
		if(GetKey(olc::Key::RIGHT).bHeld)
		{
			fCanonAngle += 2.0f * fElapsedTime;
		}
		if(GetKey(olc::Key::SPACE).bReleased)
		{
			nScore-=1;
			vecAerialBullets.push_back(std::make_unique<cPhysics>(vfCanonPos.x + 16 * cosf(fCanonAngle - 3.14592 / 2.0f), vfCanonPos.y + 16 * sinf(fCanonAngle - 3.141592 / 2.0f),fCanonAngle - 3.141592 / 2.0f,fBulletSpeed,'b'));
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
				int Trooper = 0;
				if(nScore < 200) //Difficulty scaling based on score
				{
					Trooper = rand() % 3; // 1 in 3 helicopter will have troopers
				}
				else if(nScore >= 200 && nScore < 400)
				{
					nHelicopterSpeed = 60.0f;
					Trooper = rand() % 2;
				}
				else
				{
					nHelicopterSpeed = 80.0f;
					Trooper = 0;
				}

				if(dir == 0)
				{
					float posx = 0 - i * offsetx;
					float posy = 3 + rand() % 5 + offsety * i;
					cPhysics *phy = new cPhysics(posx, posy, 0.0f, nHelicopterSpeed,'h');
					if(Trooper == 0)
					{
						phy->bIsDeploy = true;
						phy->fDeploy = rand() % (ScreenWidth() - 8);
						if(phy->fDeploy > (ScreenWidth() / 2 - 15.0f) && phy->fDeploy < (ScreenWidth() / 2 + 15.0f))
						{
							phy->fDeploy > (ScreenWidth() / 2) ? phy->fDeploy += 25.0f : phy->fDeploy -= 25.0f;
						}
					}
					vecAerialHelicopter.push_back(std::unique_ptr<cPhysics>(phy));
					nCurrentEnemy ++;
				}
				else
				{
					float posx = ScreenWidth() + offsetx;
					float posy = 3 + rand() % 5 + offsety * i;
					cPhysics *phy = new cPhysics(posx, posy, 3.141592f, nHelicopterSpeed,'h');
					if(Trooper == 0)
					{
						phy->bIsDeploy = true;
						phy->fDeploy = rand() % (ScreenWidth() - 8);
						if(phy->fDeploy > (ScreenWidth() / 2 - 15.0f) && phy->fDeploy < (ScreenWidth() / 2 + 15.0f))
						{
							phy->fDeploy > (ScreenWidth() / 2) ? phy->fDeploy += 25.0f : phy->fDeploy -= 25.0f;
						}
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
				if(f->fpx > h->fpx && f->fpx < (h->fpx + 16) && f->fpy < (h->fpy + 8) && f->fpy > h->fpy) //checs the top left position of bullet with the helicopter
				{
					f->bDead = true;
					h->bDead = true;
				}
				else if((f->fpx + 2) > h->fpx && (f->fpx + 2) < (h->fpx + 16) && f->fpy < (h->fpy + 8) && f->fpy > h->fpy) // checks the top right postion with the helicopter
				{
					f->bDead = true;
					h->bDead = true;
				}
				if(h->bDead)
				{
					nScore+=10;
				}
			}
			for( auto &h : vecAerialTroopers) //collision detection
			{
				if(f->fpx >= h->fpx && f->fpx <= (h->fpx + 8) && f->fpy <= (h->fpy + 8) && f->fpy >= h->fpy) //checs the top left position of bullet with the Trooper
				{
					f->bDead = true;
					h->bDead = true;
				}
				else if((f->fpx + 2) >= h->fpx && (f->fpx + 2) <= (h->fpx + 8) && f->fpy <= (h->fpy + 8) && f->fpy >= h->fpy) // checks the top right postion with the Troope
				{
					f->bDead = true;
					h->bDead = true;
				}
				else if((f->fpx + 2) >= h->fpx && (f->fpx + 2) <= (h->fpx + 8) && (f->fpy + 2) <= (h->fpy + 8) && (f->fpy + 2) >= h->fpy) // checks the bottom right postion with the Trooper
				{
					f->bDead = true;
					h->bDead = true;
				}
				else if(f->fpx >= h->fpx && f->fpx <= (h->fpx + 8) && (f->fpy + 2) <= (h->fpy + 8) && (f->fpy + 2) >= h->fpy) // checks the bottom right postion with the trooper
				{
					f->bDead = true;
					h->bDead = true;
				}
				bool changeHappened  = false; //flag to prevent further increase in score variable once the parachute is destroyed
				//collision detection fo the parachute
				if(h->bParachute && !h->bPrachuteDestroyed && !h->bDead)
				{
					
					if(f->fpx >= h->fpx && f->fpx <= (h->fpx + 8) && f->fpy <= (h->fpy) && f->fpy >= h->fpy - 8) //checs the top left position of bullet with the Trooper
					{
						changeHappened = true;
						f->bDead = true;
						h->bParachute = false;
						h->bPrachuteDestroyed = true;
					}
					else if((f->fpx + 2) >= h->fpx && (f->fpx + 2) <= (h->fpx + 8) && f->fpy <= (h->fpy) && f->fpy >= h->fpy - 8) // checks the top right postion with the Troope
					{
						changeHappened = true;
						f->bDead = true;
						h->bParachute = false;
						h->bPrachuteDestroyed = true;
					}
					else if((f->fpx + 2) >= h->fpx && (f->fpx + 2) <= (h->fpx + 8) && (f->fpy + 2) <= (h->fpy) && (f->fpy + 2) >= h->fpy - 8) // checks the bottom right postion with the Trooper
					{
						changeHappened = true;
						f->bDead = true;
						h->bParachute = false;
						h->bPrachuteDestroyed = true;
					}
					else if(f->fpx >= h->fpx && f->fpx <= (h->fpx + 8) && (f->fpy + 2) <= (h->fpy) && (f->fpy + 2) >= h->fpy - 8) // checks the bottom right postion with the trooper
					{
						changeHappened = true;
						f->bDead = true;
						h->bParachute = false;
						h->bPrachuteDestroyed = true;
					}
				}
				if((h->bPrachuteDestroyed && changeHappened) || h->bDead)
				{
					nScore+=10;
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
			if((int)f->fpx == (int)f->fDeploy && f->bIsDeploy)
			{
				cPhysics *troop = new cPhysics(f->fpx,f->fpy, 3.141592f / 2.0f,50.0f,'t');
				vecAerialTroopers.push_back(std::unique_ptr<cPhysics>(troop));
				f->bIsDeploy = false; 
			}
			if(f->fTotalDistanceTravelledX > ScreenWidth() + 40)
				f->bDead = true;
			if(f->fAngle == 0)
				DrawSprite(f->fpx,f->fpy,sprHelicopter1);
			if(f->fAngle == 3.141592f)
				DrawSprite(f->fpx,f->fpy,sprHelicopter2);
		}



		for(auto &f : vecAerialTroopers)
		{
			if(f->fpy >= (float)nSHeight - 8.0f && !f->bStable) //if the troopers hit the border chages the coordinates and velocity to stationary.
			{
				std::cout << f->fpx << " ";
				if(f->bPrachuteDestroyed)
					f->bDead = true;
				f->fpy = (float)nSHeight - 8.0f;
				f->fvy = 0.0f;
				f->bStable = true;
				std:: cout << vecAerialTroopers.size() << " ";
				if(f->fpx < ((float)ScreenWidth() / 2.0f) && !f->bDead)
					nEnemyLandedLeft += 1;
				else if (f->fpx >= ((float)ScreenWidth() / 2.0f) && !f->bDead)
					nEnemyLandedRight += 1;
				std::cout << nEnemyLandedLeft << " " << nEnemyLandedRight << std::endl;
			}

			f->UpdateCoordinates(fElapsedTime);
			if(!f->bStable)
			{
				if(f->fpy > nSHeight - 130.0f && !f->bPrachuteDestroyed && !f->bParachute) //draw parachute if the trooper has travelled below a particulay y value and if the trooper is not stable
				{
					f->fvy = 25.0f;
					f->bParachute = true;
				}
			
				if(!f->bPrachuteDestroyed && f->bParachute)
					DrawSprite(f->fpx,f->fpy - 8.0f,sprParachute);
				else
					f->fvy = 50.0f;
			}
			DrawSprite(f->fpx,f->fpy,sprTroopers);
		}
		SetPixelMode(olc::Pixel::NORMAL);
		
		//remove dead objects from vectors
		vecAerialBullets.remove_if([](std::unique_ptr<cPhysics> &o) {return o->bDead;});
		vecAerialHelicopter.remove_if([](std::unique_ptr<cPhysics> &o) {return o->bDead;});
		vecAerialTroopers.remove_if([](std::unique_ptr<cPhysics> &o) {return o->bDead;});



		nCurrentEnemy = vecAerialHelicopter.size();
		if(nCurrentEnemy == 0 && fTimePassed > 3.0f)
		{
			fTimePassed = 1.0f;
		}

		if(nEnemyLandedLeft >= 4)
			bGameOverLeft = true;
		if(nEnemyLandedRight >=4)
			bGameOverRight = true;
		
		//DrawBorders
		DrawLine({0,nSHeight + 1}, { nSWidth, nSHeight + 1 }, olc::BLUE);
		// called once per frame
		FillRect(viBaseRectPos, {30 , 30}, olc::GREY);

		//Draw score
		DrawString({10,nSHeight + 3},"Score: ");
		DrawString({55,nSHeight + 3},std::to_string(nScore));

		DrawDecal(vfBasePos,decBase);
		DrawRotatedDecal(vfCanonPos,decCanon,fCanonAngle,{0.0f,15.0f},{1.0f,1.0f},olc::CYAN);
		}

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
