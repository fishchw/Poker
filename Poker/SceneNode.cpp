/*!
 * \file SceneNode.cpp
 *
 * \author LYG
 * \date ���� 2018
 *
 * 
 */
#include "SceneNode.h"

SceneNode::SceneNode()
{
	mPosition = Vector3::ZERO;
	mRotation = Quaternion::IDENTITY;
}

SceneNode::~SceneNode()
{

}