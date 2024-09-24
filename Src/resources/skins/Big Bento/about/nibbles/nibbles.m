/*---------------------------------------------------
-----------------------------------------------------
Filename:	nibbles.m
Version:	1.0

Type:		maki
Date:		23. Okt. 2006 - 21:11 
Author:		Martin Poehlmann aka Deimos
E-Mail:		martin@skinconsortium.com
Internet:	www.skinconsortium.com
		www.martin.deimos.de.vu
-----------------------------------------------------
---------------------------------------------------*/

#define GAME_SPEED 30
#define MAX_H 59
#define MAX_W 72
#define MAX_LLAMAS 9

function init_nibbles();

Function setLlama();
Function gotLlama();
Function showInfo(string s);
Function showInfo2();
Function hideInfo2();
Function startGame(int level);
Function setPos(layer l, int x, int y);
Function checkforWalls(layer l);
Function checkforLlama(layer l);
Function checkforSammy(layer l);
Function sammyDies ();
Function addSammy();
Function syncSammy();
Function Layer createSammy(int num, int x, int y);

Global group nibbles;

Global Layer bg, llama, info, info2;
Global Text infotxt, infotxt2a, infotxt2b, infotxt2c, infotxt2d;

Global Layer sammy0;
Global String gamestate = "startup";
Global String direction;
Global Boolean gotkey = 0;
Global Timer move;
Global Boolean paused;

Global map wall;

Global int s_score, s_lives, s_level, currentllama;
Global Text t_score, t_lives, t_level;

Global int n_sammy, expn_sammy;
Global List sammy, sammy_x, sammy_y;
Global Group sammys;

Global int cheat, nocheat;
Global int cheat2, nocheat2;
Global int cheat3, nocheat3;
Global int evercheat;

Global Text c1, c2, c3;

init_nibbles()
{
	sammy = new List;
	sammy_x = new List;
	sammy_y = new List;

	llama = nibbles.getObject("llama");
	sammy0 = nibbles.getObject("sammy0");
	info = nibbles.getObject("info");
	infotxt = nibbles.getObject("infotxt");
	sammys = nibbles.getObject("sammys");
	bg = nibbles.getObject("bg");

	info2 = nibbles.getObject("info2");
	infotxt2a = nibbles.getObject("infotxt2a");
	infotxt2b = nibbles.getObject("infotxt2b");
	infotxt2c = nibbles.getObject("infotxt2c");
	infotxt2d = nibbles.getObject("infotxt2d");
	
	t_lives = nibbles.getObject("lives");
	t_score = nibbles.getObject("score");
	t_level = nibbles.getObject("level");
	c1 = nibbles.getObject("c1");
	c2 = nibbles.getObject("c2");
	c3 = nibbles.getObject("c3");
	gamestate = "startup";

	n_sammy = 0;
	expn_sammy = 0;
	s_level = 1;
	s_score = 0;
	s_lives = 5;

	cheat = 0;
	nocheat = 0;
	cheat2 = 0;
	nocheat2 = 0;
	cheat3 = 0;
	nocheat3 = 0;
	evercheat = 0;

	move = new Timer;
	move.setDelay(GAME_SPEED);
}

System.onKeyDown (String key)
{
	if (nibbles.isVisible() && nibbles.isActive())
	{
		if (!gotkey)
		{
			if (strsearch(key, "up") != -1 && direction != "d" && direction != "u")
			{
				if (move.isRunning())
				{
					gotkey = 1;
					direction = "u";
					complete;
					return;
				}
			}
			else if (strsearch(key, "down") != -1 && direction != "u" && direction != "d")
			{
				if (move.isRunning())
				{
					gotkey = 1;
					direction = "d";
					complete;
					return;
				}
			}
			else if (strsearch(key, "left") != -1 && direction != "r" && direction != "l")
			{
				if (move.isRunning())
				{
					gotkey = 1;
					direction = "l";
					complete;
					return;
				}
			}
			else if (strsearch(key, "right") != -1 && direction != "l" && direction != "r")
			{
				if (move.isRunning())
				{
					gotkey = 1;
					direction = "r";
					complete;
					return;
				}
			}			
		}


		/* Cheet on */

		if (key == "n" && cheat == 0 && cheat3 != 4 && nocheat3 != 4 && cheat3 != 1 && nocheat3 != 1)
		{
			cheat = 1;
			complete;
			return;
		}
		else if (key == "o" && cheat == 1)
		{
			cheat = 2;
			complete;
			return;
		}
		else if (key == "s" && cheat == 2)
		{
			cheat = 3;
			complete;
			return;
		}
		else if (key == "a" && cheat == 3)
		{
			cheat = 4;
			complete;
			return;
		}
		else if (key == "m" && cheat == 4 && cheat != 5)
		{
			cheat = 5;
			complete;
			return;
		}
		else if (key == "m" && cheat == 5 && cheat != 4)
		{
			cheat = 6;
			complete;
			return;
		}
		else if (key == "y" && cheat == 6)
		{
			c1.setText("NOSAMMY");
			cheat = 7;
			evercheat = 1;
			complete;
			return;
		}

		/* Cheet off */

		else if (key == "n" && cheat == 7 && cheat3 != 4 && nocheat3 != 4 && cheat3 != 1 && nocheat3 != 1)
		{
			nocheat = 1;
			complete;
			return;
		}
		else if (key == "o" && nocheat == 1)
		{
			nocheat = 2;
			complete;
			return;
		}
		else if (key == "s" && nocheat == 2)
		{
			nocheat = 3;
			complete;
			return;
		}
		else if (key == "a" && nocheat == 3)
		{
			nocheat = 4;
			complete;
			return;
		}
		else if (key == "m" && nocheat == 4 && nocheat != 5)
		{
			nocheat = 5;
			complete;
			return;
		}
		else if (key == "m" && nocheat == 5 && nocheat != 4)
		{
			nocheat = 6;
			complete;
			return;
		}
		else if (key == "y" && nocheat == 6  && nocheat3 != 7)
		{
			c1.setText("");
			cheat = 0;
			nocheat = 0;
			complete;
			return;
		}

		/* end cheat */


		/* Cheet2 on */

		else if (key == "f" && cheat2 == 0 && cheat3 != 2 && nocheat3 != 2)
		{
			cheat2 = 1;
			complete;
			return;
		}
		else if (key == "r" && cheat2 == 1)
		{
			cheat2 = 2;
			complete;
			return;
		}
		else if (key == "e" && cheat2 == 2 && cheat2 != 3)
		{
			cheat2 = 3;
			complete;
			return;
		}
		else if (key == "e" && cheat2 == 3 && cheat2 != 2)
		{
			cheat2 = 4;
			complete;
			return;
		}
		else if (key == "s" && cheat2 == 4)
		{
			cheat2 = 5;
			complete;
			return;
		}
		else if (key == "a" && cheat2 == 5)
		{
			cheat2 = 6;
			complete;
			return;
		}
		else if (key == "m" && cheat2 == 6)
		{
			c2.setText("FREESAM");
			cheat2 = 7;
			evercheat = 1;
			complete;
			return;
		}

		/* Cheet2 off */

		else if (key == "f" && cheat2 == 7 && cheat3 != 2 && nocheat3 != 2)
		{
			nocheat2 = 1;
			complete;
			return;
		}
		else if (key == "r" && nocheat2 == 1)
		{
			nocheat2 = 2;
			complete;
			return;
		}
		else if (key == "e" && nocheat2 == 2 && nocheat2 != 3)
		{
			nocheat2 = 3;
			complete;
			return;
		}
		else if (key == "e" && nocheat2 == 3 && nocheat2 != 2)
		{
			nocheat2 = 4;
			complete;
			return;
		}
		else if (key == "s" && nocheat2 == 4)
		{
			nocheat2 = 5;
			complete;
			return;
		}
		else if (key == "a" && nocheat2 == 5)
		{
			nocheat2 = 6;
			complete;
			return;
		}
		else if (key == "m" && nocheat2 == 6)
		{
			c2.setText("");
			cheat2 = 0;
			nocheat2 = 0;
			complete;
			return;
		}

		/* end cheat2 */

		/* Cheet3 on */

		else if (key == "i" && cheat3 == 0 && cheat3 != 3 && cheat3 != 5)
		{
			cheat3 = 1;
			complete;
			return;
		}
		else if (key == "n" && cheat3 == 1 && cheat3 != 4)
		{
			cheat3 = 2;
			complete;
			return;
		}
		else if (key == "f" && cheat3 == 2)
		{
			cheat3 = 3;
			complete;
			return;
		}
		else if (key == "i" && cheat3 == 3 && cheat3 != 5 && cheat3 != 0)
		{
			cheat3 = 4;
			complete;
			return;
		}
		else if (key == "n" && cheat3 == 4 && cheat3 != 2)
		{
			cheat3 = 5;
			complete;
			return;
		}
		else if (key == "i" && cheat3 == 5 && cheat3 != 3 && cheat3 != 0)
		{
			cheat3 = 6;
			complete;
			return;
		}
		else if (key == "t" && cheat3 == 6)
		{
			cheat3 = 7;
			complete;
			return;
		}
		else if (key == "y" && cheat3 == 7)
		{
			c3.setText("INFINITY");
			cheat3 = 8;
			evercheat = 1;
			complete;
			return;
		}

		/* Cheet3 off */

		else if (key == "i" && cheat3 == 8 && nocheat3 != 3 && nocheat3 != 5)
		{
			nocheat3 = 1;
			complete;
			return;
		}
		else if (key == "n" && nocheat3 == 1 && nocheat3 != 4)
		{
			nocheat3 = 2;
			complete;
			return;
		}
		else if (key == "f" && nocheat3 == 2)
		{
			nocheat3 = 3;
			complete;
			return;
		}
		else if (key == "i" && nocheat3 == 3 && nocheat3 != 5 && nocheat3 != 0)
		{
			nocheat3 = 4;
			complete;
			return;
		}
		else if (key == "n" && nocheat3 == 4 && nocheat3 != 1)
		{
			nocheat3 = 5;
			complete;
			return;
		}
		else if (key == "i" && nocheat3 == 5 && nocheat3 != 3 && nocheat3 != 0)
		{
			nocheat3 = 6;
			complete;
			return;
		}
		else if (key == "t" && nocheat3 == 6)
		{
			nocheat3 = 7;
			complete;
			return;
		}
		else if (key == "y" && nocheat3 == 7 && nocheat != 6)
		{
			c3.setText("");
			cheat3 = 0;
			nocheat3 = 0;
			complete;
			return;
		}

		/* end cheat3 */

		else if (key == "space")
		{
			if (gamestate == "startup" || gamestate == "New Try" || gamestate == "Next level")
			{
				hideInfo2();
				info.hide();
				infotxt.hide();
				gamestate = "running";
				startGame(s_level);
				complete;
				return;
			}
			if (gamestate == "Game Over")
			{
				info.hide();
				infotxt.hide();
				showInfo2();
				s_level = 1;
				complete;
				return;
			}
			if (gamestate == "Once Again")
			{
				s_level = 1;
				s_score = 0;
				s_lives = 5;
				hideInfo2();
				info.hide();
				infotxt.hide();
				gamestate = "running";
				startGame(s_level);
				complete;
				return;
			}
			if (move.isRunning())
			{
				paused = 1;
				move.stop();
				showInfo("Game Paused ... Push Space");
				complete;
				return;
			}
			if (paused == 1)
			{
				paused = 0;
				info.hide();
				infotxt.hide();
				move.start();
				complete;
				return;
			}
		}
		else
		{
			complete;
			return;
		}
	/*	if (key == "up" && direction != "d")
		{
			if (move.isRunning())
			{
				direction = "u";
				complete;
				return;
			}
		}
		if (key == "down" && direction != "u")
		{
			if (move.isRunning())
			{
				direction = "d";
				complete;
				return;
			}
		}
		if (key == "left" && direction != "r")
		{
			if (move.isRunning())
			{
				direction = "l";
				complete;
				return;
			}
		}
		if (key == "right" && direction != "l")
		{
			if (move.isRunning())
			{
				direction = "r";
				complete;
				return;
			}
		}*/
	}
}

startGame (int level)
{
	if (level == 11) level = 10;

	if (wall) delete wall;
	wall = new map;
	wall.loadMap("level" + integerToString(level));
	bg.setXmlParam("image", "level" + integerToString(level));

	int n = sammy.getNumItems();

	for ( int i = 1;  i <= n; i++ )
	{
		layer newsammy = sammy.enumItem(0);
		sammy.removeItem(0);
		newsammy.hide();			
	}	

	currentllama = 0;
	t_lives.setText(translate("Lives: ") + integerToString(s_lives));
	t_score.setText(translate("Score: ") + integerToString(s_score));
	t_level.setText(translate("Level: ") + integerToString(s_level) + " [" + integerToString(currentllama) + "/" + integerToString(MAX_LLAMAS) + "]");
	n_sammy = 0;
	expn_sammy = 5;
	gotkey = 1;
	if (level != 8) direction = "r";
	else direction = "u";
	setLlama();
	sammy.removeAll();
	sammy_x.removeAll();
	sammy_y.removeAll();
	sammy0.setXmlParam("x", "180");
	sammy0.setXMLParam("y", "155");
	sammy0.show();
	move.start();
}

move.onTimer ()
{
	if (n_sammy < expn_sammy)
	{
		addSammy();
	}
	else
	{
		syncSammy();
	}

	if (direction == "r")
	{
		setPos(sammy0, 1, 0);
	}
	else if (direction == "l")
	{
		setPos(sammy0, -1, 0);
	}
	else if (direction == "u")
	{
		setPos(sammy0, 0, -1);
	}
	else if (direction == "d")
	{
		setPos(sammy0, 0, 1);
	}
	checkForWalls (sammy0);
	checkForSammy (sammy0);
	checkForllama (sammy0);
}

setPos (layer l, int x, int y)
{
	l.setXMLParam("x", integerToString(l.getGuiX() + x*5));
	l.setXMLParam("y", integerToString(l.getGuiY() + y*5));
	gotkey = 0;
}

setLlama ()
{
	int lx = 5+random(MAX_W-3)*5;
	int ly = 20+random(MAX_H-3)*5;

	int x = sammy0.getGuiX();
	int y = sammy0.getGuiY();

	if ((x == lx || x == lx + 5 || x == lx + 10) && (y == ly || y == ly + 5 || y == ly + 10))
	{
		setLlama ();
		return;
	}

	if (wall.getValue(lx, ly) == 255 || wall.getValue(lx+5, ly) == 255 || wall.getValue(lx+10, ly) == 255 ||
	    wall.getValue(lx, ly+5) == 255 || wall.getValue(lx+5, ly+5) == 255 || wall.getValue(lx+10, ly+5) == 255 ||
	    wall.getValue(lx, ly+10) == 255 || wall.getValue(lx+5, ly+10) == 255 || wall.getValue(lx+10, ly+10) == 255 )
	{
		setLlama ();
		return;
	}

	int n = sammy_x.getNumItems();

	for ( int i = 0;  i < n; i++ )
	{
		x = sammy_x.enumItem(i);
		y = sammy_y.enumItem(i);
		if ((x == lx || x == lx + 5 || x == lx + 10) && (y == ly || y == ly + 5 || y == ly + 10))
		{
			setLlama ();
			return;
		}
	}

	if ((x == lx || x == lx + 5 || x == lx + 10) && (y == ly || y == ly + 5 || y == ly + 10)) gotLlama();

	llama.setXMLParam("x", integerToString(lx));
	llama.setXMLParam("y", integerToString(ly));

	llama.show();
}

addSammy ()
{
	int x = sammy0.getGuiX();
	int y = sammy0.getGuiY();
	n_sammy++;
	layer newsammy = createSammy(n_sammy, x, y);
	sammy.addItem(newsammy);
	sammy_x.addItem(x);
	sammy_y.addItem(y);
	newsammy.setXMLParam("x", integerToString(x));
	newsammy.setXMLParam("y", integerToString(y));
	newsammy.show();
}

Layer createSammy (int num, int x, int y)
{
	layer l = new Layer;
	l.setXmlParam("id", "sammy" + integerToString(num));
	l.setXmlParam("image", "sammy");
	l.setXMLParam("x", integerToString(x));
	l.setXMLParam("y", integerToString(y));
	l.init(sammys);
	return l;
}

syncSammy ()
{
	layer newsammy = sammy.enumItem(0);
	sammy.removeItem(0);
	sammy_x.removeItem(0);
	sammy_y.removeItem(0);
	int x = sammy0.getGuiX();
	int y = sammy0.getGuiY();
	newsammy.setXMLParam("x", integerToString(x));
	newsammy.setXMLParam("y", integerToString(y));
	sammy.addItem(newsammy);
	sammy_x.addItem(x);
	sammy_y.addItem(y);
}

showInfo (string s)
{
	infotxt.setText(s);
	info.show();
	infotxt.show();
}

checkForWalls (layer l)
{
	if ( cheat2 == 7 ) return;
	int x = l.getGuiX();
	int y = l.getGuiY();

	if (wall.getValue(x, y) == 255) sammyDies();
}

checkForLlama (layer l)
{
	int x = l.getGuiX();
	int y = l.getGuiY();

	int lx = llama.getGuiX();
	int ly = llama.getGuiY();

	if ((x == lx || x == lx + 5 || x == lx + 10) && (y == ly || y == ly + 5 || y == ly + 10)) gotLlama();
}

checkForSammy (layer l)
{
	if ( cheat == 7 ) return;
	int x = l.getGuiX();
	int y = l.getGuiY();
	int n = sammy_x.getNumItems();

	for ( int i = 0;  i < n; i++ )
	{
		if (sammy_x.enumItem(i) == x && sammy_y.enumItem(i) == y)
		{
			sammyDies ();
		}
	}
}

sammyDies ()
{
	llama.hide();
	move.stop();
	sammy0.hide();
	s_lives--;
	s_score -= 1000;
	t_lives.setText(translate("Lives: ") + integerToString(s_lives));
	t_score.setText(translate("Score: ") + integerToString(s_score));
	t_level.setText(translate("Level: ") + integerToString(s_level) + " [" + integerToString(currentllama) + "/" + integerToString(MAX_LLAMAS) + "]");
	gamestate = "New Try";
	if (s_lives == 0) {
		gamestate = "Game Over";
	}
	showInfo ("Sammy Dies! Push Space!");
}

gotLlama ()
{
	currentllama++;
	s_score += 100 * currentllama;
	t_score.setText(translate("Score: ") + integerToString(s_score));
	t_level.setText(translate("Level: ") + integerToString(s_level) + " [" + integerToString(currentllama) + "/" + integerToString(MAX_LLAMAS) + "]");
	if (currentllama >= MAX_LLAMAS && cheat3 != 8)
	{
		move.stop();
		s_level++;
		llama.hide();
		gamestate = "Next Level";
		showInfo (translate("Level: ") + integerToString(s_level) + translate(",  Push Space"));	
	}
	else
	{
		expn_sammy += 12;
		setLlama();
	}
}

showInfo2 ()
{
	if (evercheat == 0)
	{
		if (s_score > getPrivateInt("Nibbles", "Personal Best", 0))
		{
			setPrivateInt("Nibbles", "Personal Best", s_score);
			infotxt2d.setText("Congrats, New Highscore!");
		}
		else infotxt2d.setText(translate("Highscore") +": " + integerToString(getPrivateInt("Nibbles", " ", 0)));
	}
	else
	{
		infotxt2d.setTExt("You have cheated!");
	}
	if (cheat == 0 && cheat2 == 0 && cheat3 ==0) evercheat = 0;
	gamestate = "Once Again";
	infotxt2b.setText(translate("Your Score") + ": " + integerToString(s_score));
	info2.show();
	infotxt2a.show();
	infotxt2d.show();
	infotxt2b.show();
	infotxt2c.show();
}

hideInfo2 ()
{
	info2.hide();
	infotxt2a.hide();
	infotxt2b.hide();
	infotxt2c.hide();
	infotxt2d.hide();
}