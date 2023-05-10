#pragma once

//���ő啝
const int kMaxWidth = 50;
//�c�ő啝
const int kMaxHeight = 25;

//�u���b�N�̎��
const int kMaxBlockType = 5;

//�}�b�v�̑�����s���N���X
class Map
{
public:
	Map();
	~Map();

	void Update();
	void Draw();

private:

	//csv���������Ɏg��������
	char string[kMaxBlockType][2] = { "0", "1", "2", "3", "4" };

	//�}�b�v�`�b�v�̐�
	int map[kMaxHeight][kMaxWidth];

	//�}�E�XX���W
	int mouseX;
	//�}�E�XY���W
	int mouseY;

	//�ݒu����u���b�N�̃i���o�[
	int blockNum;

};


