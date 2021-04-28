
# include <Siv3D.hpp>

enum class Mode
{
	LOAD_SELECT,
	SELECT_STAGE,
	LOAD_STAGE,
	PLAY,
	CLEAR,
	OVER,
	EDIT,
};

struct Target
{
	Vec2 pos;
	bool value;
};

void Main()
{
	bool allClear = false;
	Mode mode = Mode::LOAD_SELECT;
	int stage = 1;
	int maxStage = 1;
	Array<bool> isCleared;

	Color color[3] = { Palette::White, Palette::Black, Palette::Darkcyan };
	const int width = 800, height = 600;

	bool background[height * height];
	bool foreground[height * height];
	bool isSelecting = false;

	Array<Target> target;

	unsigned int point = 0;
	int visualRectCount = 0;
	int finalRectCount = 0;
	int maxRectCount = 0;

	Window::Resize(width, height);
	Scene::SetBackground(color[2]);

	Image image(height, height, Palette::White);
	DynamicTexture texture(image);
	Rect field(width - height, 0, height, height);

	Vec2 mouseInitialPos;

	const Font font16(16);
	const Font font60(60);

	String caption = U"";

	const Rect nextButton(0, 400, width - height, 50);
	const Rect saveSelectButton(0, 450, width - height, 50);
	const Rect clearButton(0, 500, width - height, 50);
	const Rect modeButton(0, 550, width - height, 50);

	const Rect nextGroupButton(width - height, height - 20, height, 20);
	const Rect prevGroupButton(width - height, 0, height, 20);

	int group = 0;

	for (size_t i = 0; i < height * height; i++)
	{
		background[i] = false;
		foreground[i] = false;
	}

	BinaryWriter w(U"data/stage.bin", OpenMode::Append);
	w.setPos(0);
	w.write((int)16);
	w.close();

	while (System::Update())
	{
		if (mode == Mode::EDIT)
		{
			if (MouseR.down())
			{
				target.push_back({ Cursor::Pos(), false });
			}

			if (saveSelectButton.leftClicked())
			{
				BinaryWriter writer(U"data/stage.bin", OpenMode::Append);
				writer.write(stage);
				writer.setPos(stage + 3);
				writer.write(false);
				CSVData csv;
				csv.write(finalRectCount);
				csv.write(false);
				for (size_t i = 0; i < target.size(); i++)
				{
					csv.newLine();
					csv.write(target[i].pos.x - width + height);
					csv.write(target[i].pos.y);
					csv.write(target[i].value);
				}
				csv.save(Format(U"data/stage", stage, U".csv"));
			}
		}

		if (mode == Mode::PLAY || mode == Mode::EDIT)
		{
			if (field.leftClicked())
			{
				mouseInitialPos = Cursor::Pos();
				visualRectCount++;
				isSelecting = true;
			}

			if (isSelecting)
			{
				if (MouseL.pressed())
				{
					for (size_t i = 0; i < height * height; i++)
					{
						foreground[i] = background[i];
					}
					for (int x = Max(0, Min((int)mouseInitialPos.x, (int)Cursor::Pos().x) - width + height); x < Min(height, Max((int)mouseInitialPos.x, (int)Cursor::Pos().x) - width + height); x++)
					{
						for (int y = Max(0, Min((int)mouseInitialPos.y, (int)Cursor::Pos().y)); y < Min(height, Max((int)mouseInitialPos.y, (int)Cursor::Pos().y)); y++)
						{
							foreground[y * height + x] = !background[y * height + x];
						}
					}
					for (size_t x = 0; x < height; x++)
					{
						for (size_t y = 0; y < height; y++)
						{
							image[y][x] = (foreground[y * height + x] ? color[1] : color[0]);
						}
					}
					texture.fill(image);
				}

				if (MouseL.up())
				{
					for (size_t i = 0; i < height * height; i++)
					{
						background[i] = foreground[i];
					}

					isSelecting = false;
					finalRectCount++;
				}
			}
		}

		if (mode == Mode::EDIT)
		{
			for (size_t i = 0; i < target.size(); i++)
			{
				target[i].value = foreground[(int)target[i].pos.y * height + (int)target[i].pos.x - width + height];
			}
		}
		else if (mode == Mode::PLAY)
		{
			point = 0;
			for (size_t i = 0; i < target.size(); i++)
			{
				if (target[i].value == foreground[(int)target[i].pos.y * height + (int)target[i].pos.x - width + height])
				{
					point++;
				}
			}
		}

		if (mode == Mode::PLAY && !isSelecting)
		{
			if (point == target.size())
			{
				mode = Mode::CLEAR;
				BinaryWriter writer(Format(U"data/stage.bin"), OpenMode::Append);
				writer.setPos(stage + 3);
				writer.write(true);
			}
			else if (finalRectCount == maxRectCount)
			{
				mode = Mode::OVER;
			}
		}

		if (mode == Mode::LOAD_SELECT)
		{
			BinaryReader reader(Format(U"data/stage.bin"));
			isCleared.clear();
			reader.read(maxStage);
			for (int i = 0; i < maxStage; i++)
			{
				bool _isCleared;
				reader.read(_isCleared);
				isCleared.push_back(_isCleared);
			}
			mode = Mode::SELECT_STAGE;
		}

		if (mode == Mode::SELECT_STAGE)
		{
			field.draw(color[0]);
			for (int i = 0; i < Min(maxStage - group * 16, 16); i++)
			{
				Rect stageRect(width - height + 40 + 140 * (i % 4), 40 + 140 * (i / 4), 100, 100);
				stageRect.draw(color[2]);
				if (stageRect.mouseOver())
				{
					stageRect.draw(Alpha(64));
				}
				if (stageRect.leftClicked())
				{
					stage = group * 16 + i + 1;
					mode = Mode::LOAD_STAGE;
				}
				font16(group * 16 + i + 1).drawAt(stageRect.x + 50, stageRect.y + 50, isCleared[i] ? Palette::Darkgray : Palette::White);

				if (group > 0)
				{
					prevGroupButton.draw(color[2]);
					if (prevGroupButton.mouseOver())
					{
						prevGroupButton.draw(Alpha(64));
					}
					if (prevGroupButton.leftClicked())
					{
						group--;
					}
					font16(U"go back").drawAt(prevGroupButton.x + 100, prevGroupButton.y + 10, color[0]);
				}

				if (16 * (group + 1) < maxStage)
				{
					nextGroupButton.draw(color[2]);
					if (nextGroupButton.mouseOver())
					{
						nextGroupButton.draw(Alpha(64));
					}
					if (nextGroupButton.leftClicked())
					{
						group++;
					}
					font16(U"go forward").drawAt(nextGroupButton.x + 100, nextGroupButton.y + 10, color[0]);
				}
			}
		}

		if ((mode == Mode::CLEAR && nextButton.leftClicked()) || clearButton.leftClicked() || modeButton.leftClicked() || ((mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER) && saveSelectButton.leftClicked()))
		{
			for (size_t i = 0; i < height * height; i++)
			{
				background[i] = false;
				foreground[i] = false;
			}
			if (modeButton.leftClicked() || mode == Mode::EDIT || ((mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER) && saveSelectButton.leftClicked()))
			{
				target.clear();
			}
			visualRectCount = 0;
			finalRectCount = 0;
			for (size_t x = 0; x < height; x++)
			{
				for (size_t y = 0; y < height; y++)
				{
					image[y][x] = color[0];
				}
			}
			texture.fill(image);
		}

		if (mode == Mode::CLEAR && nextButton.leftClicked())
		{
			if (stage < maxStage)
			{
				stage++;
				mode = Mode::LOAD_STAGE;
			}
			else
			{
				allClear = true;
			}
		}

		if (mode == Mode::LOAD_STAGE)
		{
			visualRectCount = 0;
			finalRectCount = 0;
			isSelecting = false;
			CSVData reader(Format(U"data/stage", stage, U".csv"));
			maxRectCount = reader.get<int>(0, 0);
			target.clear();
			for (size_t j = 1; j < reader.rows(); j++)
			{
				target.push_back({ { reader.get<double>(j, 0) + width - height, reader.get<double>(j, 1) }, reader.get<bool>(j, 2) });
			}
			mode = Mode::PLAY;
		}

		if (clearButton.leftClicked())
		{
			if (mode == Mode::CLEAR || mode == Mode::OVER)
			{
				mode = Mode::PLAY;
			}
		}
		if (modeButton.leftClicked())
		{
			if (mode == Mode::EDIT)
			{
				mode = Mode::LOAD_SELECT;
			}
			else
			{
				mode = Mode::EDIT;
				BinaryReader reader(U"data/stage.bin");
				reader.read(stage);
				stage++;
			}
		}

		if ((mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER) && saveSelectButton.leftClicked())
		{
			mode = Mode::LOAD_SELECT;
		}

		if (mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER || mode == Mode::EDIT)
		{
			texture.draw(width - height, 0);
		}

		for (size_t i = 0; i < target.size(); i++)
		{
			Circle(target[i].pos.x, target[i].pos.y, 6).draw(target[i].value ? color[0] : color[1]);
			Circle(target[i].pos.x, target[i].pos.y, 5).draw(target[i].value ? color[1] : color[0]);
		}

		font60(U"XOR").draw(10, 10, color[0]);

		if (mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER || mode == Mode::EDIT)
		{
			font16(Format(U"Stage", stage, U", ", visualRectCount, U"/", maxRectCount)).draw(15, 120, color[0]);
		}

		switch (mode)
		{
		case Mode::PLAY:
			caption = U"ドラッグして\n矩形選択すると\n選択範囲の白と\n黒が反転します。\n白丸が白い範囲，\n黒丸が黒い範囲に\n入るようにして\nください。";
			break;

		case Mode::CLEAR:
			if (allClear)
			{
				caption = U"ALL CLEAR\nおめでとう\nございます。\n全てのステージを\nクリアしました。";
			}
			else
			{
				caption = Format(U"GAME CLEAR\nおめでとう\nございます。\nステージ", stage, U"を\nクリアしました。");
			}
			break;

		case Mode::OVER:
			caption = U"GAME OVER\n選択回数が\n上限に達し\nましたが，\n全ての丸と\n範囲の色が\n一致しませ\nんでした。";
			break;

		case Mode::EDIT:
			caption = U"ステージを\n作成します。\nドラッグで\n白黒反転，\n右クリックで\n丸の設置を\n行えます。";
			break;

		case Mode::SELECT_STAGE:
			caption = U"ステージを\n選択して\nください。\n数字が灰色の\nレベルは\nクリア済み\nです。";
			break;

		default:
			break;
		}
		font16(caption).draw(15, 170, color[0]);

		if (mode == Mode::CLEAR)
		{
			if (nextButton.mouseOver() && !isSelecting)
			{
				nextButton.draw(Alpha(64));
			}
			font16(U"Next Stage").draw(nextButton.pos.x + 15, nextButton.pos.y + 10, color[0]);
		}
		if (mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER || mode == Mode::EDIT)
		{
			if (saveSelectButton.mouseOver() && !isSelecting)
			{
				saveSelectButton.draw(Alpha(64));
			}
			if (mode == Mode::EDIT)
			{
				font16(U"Save Stage").draw(saveSelectButton.pos.x + 15, saveSelectButton.pos.y + 10, color[0]);
			}
			else
			{
				font16(U"Select Stage").draw(saveSelectButton.pos.x + 15, saveSelectButton.pos.y + 10, color[0]);
			}
		}

		if (mode == Mode::PLAY || mode == Mode::CLEAR || mode == Mode::OVER || mode == Mode::EDIT)
		{
			if (clearButton.mouseOver() && !isSelecting)
			{
				clearButton.draw(Alpha(64));
			}
			if (mode == Mode::EDIT)
			{
				font16(U"Clear Stage").draw(clearButton.pos.x + 15, clearButton.pos.y + 10, color[0]);
			}
			else
			{
				font16(U"Retry").draw(clearButton.pos.x + 15, clearButton.pos.y + 10, color[0]);
			}
		}

		if (modeButton.mouseOver() && !isSelecting)
		{
			modeButton.draw(Alpha(64));
		}
		if (mode == Mode::EDIT)
		{
			font16(U"Quit Editing").draw(modeButton.pos.x + 15, modeButton.pos.y + 10, color[0]);
		}
		else
		{
			font16(U"Edit Stage").draw(modeButton.pos.x + 15, modeButton.pos.y + 10, color[0]);
		}
	}
}
