/*!
 * \file Texture.cpp
 *
 * \author LYG
 * \date ���� 2018
 *
 * 
 */
#include "Texture.h"
#include "Scene.h"
#include <stdio.h>

D3d11Texture::D3d11Texture()
{
	mTexture = NULL;
	mRenderTargetView = NULL;
	mShaderResourceView = NULL;
	mDepthBuffer = NULL;
	mSurface = nullptr;
	mCpuAccess = false;
}

bool D3d11Texture::Initialise(std::string Name, ID3D11Device* Device, int Width, int Height, DXGI_FORMAT Format, int MipLevel, bool IsRenderTarget, bool CPUAccess)
{
	HRESULT err;
	D3D11_TEXTURE2D_DESC td;
	memset(&td, 0, sizeof(td));
	td.Width = Width;
	td.Height = Height;
	td.MipLevels = MipLevel;
	td.ArraySize = 1;
	td.Format = Format;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	if (CPUAccess)
	{
		td.Usage = D3D11_USAGE_DYNAMIC;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		td.Usage = D3D11_USAGE_DEFAULT;
		td.CPUAccessFlags = 0;
	}

	if (IsRenderTarget)
	{
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.MipLevels = 1;					// must be 1
	}
	else
	{
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}
	td.MiscFlags = 0;

	ID3D11Texture2D *texVal;
	if (FAILED(err = Device->CreateTexture2D(&td, NULL, &texVal)))
	{
		OutputDebugStringA(("D3d11Texture::CreateRenderTarget: CreateTexture2D failed"));
		return false;
	}

	//------------------------------------------
	ID3D11RenderTargetView *view = NULL;
	if (IsRenderTarget)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Format = Format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		err = Device->CreateRenderTargetView(texVal, &desc, &view);
		if (FAILED(err))
		{
			SAFE_RELEASE(texVal);
			//		SAFE_RELEASE(resource);
			OutputDebugStringA(("D3d11Texture::CreateRenderTarget: CreateRenderTargetView failed"));
			return false;
		}
		mDepthBuffer = new DepthBuffer(Device, Width, Height);
	}

	D3D11_TEXTURE2D_DESC TextureDesc;
	texVal->GetDesc(&TextureDesc);
	D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
	memset(&resourceDesc, 0, sizeof(resourceDesc));
	resourceDesc.Format = TextureDesc.Format;
	resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceDesc.Texture2D.MipLevels = 1;
	resourceDesc.Texture2D.MostDetailedMip = 0;
	if (FAILED(Device->CreateShaderResourceView(texVal, &resourceDesc, &mShaderResourceView)))
	{
		OutputDebugStringA(("D3d11Texture::CreateRenderTarget: CreateShaderResourceView failed"));
	}

	mTexture = texVal;
	mRenderTargetView = view;
	mName = Name;
	// If a texture is load from disk, it can't be access by the CPU, because it may be a DDS file ,not a bitmap.
	// So, mCpuAccess been set to false unless the texture is create in memory.
	mCpuAccess = CPUAccess;
	return true;
}

D3d11Texture::~D3d11Texture()
{
	SAFE_RELEASE(mTexture);
	SAFE_RELEASE(mRenderTargetView);
	SAFE_RELEASE(mShaderResourceView);
	SAFE_DELETE(mDepthBuffer);
}

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{

}

D3d11Texture* TextureManager::CreateRenderTarget(std::string Name, ID3D11Device* Device, int Width, int Height, DXGI_FORMAT format)
{
	if (mTextureArray.find(Name) != mTextureArray.end())
	{
		return mTextureArray[Name];
	}
	D3d11Texture* target = new D3d11Texture();
	if (target->Initialise(Name, Device, Width, Height, format, 1, true, false) == false)
	{
		SAFE_DELETE(target);
		return NULL;
	}
	mTextureArray[Name] = target;
	//
	return target;
}

D3d11Texture* TextureManager::CreateTexture(std::string Name, ID3D11Device* Device, int Width, int Height, DXGI_FORMAT format /* = DXGI_FORMAT_B8G8R8A8_UNORM */, int MipLevel /* = 1 */, bool CPUAccess/* = false*/)
{
	if (mTextureArray.find(Name) != mTextureArray.end())
	{
		return mTextureArray[Name];
	}
	D3d11Texture* texture = new D3d11Texture();
	if (texture->Initialise(Name, Device, Width, Height, format, MipLevel, false, CPUAccess) == false)
	{
		SAFE_DELETE(texture);
		return NULL;
	}
	mTextureArray[Name] = texture;
	//
	return texture;
}

D3d11Texture* TextureManager::LoadTextureFromFile(std::string Name, ID3D11Device* Device, const char* FullFileName, bool IsDDS)
{
	if (mTextureArray.find(Name) != mTextureArray.end())
	{
		return mTextureArray[Name];
	}
	FILE* fp = NULL;
	fopen_s(&fp, FullFileName, "rb");
	if (fp == NULL)
	{
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	int FileLength = ftell(fp);
	rewind(fp);
	unsigned char* FileBuffer = new unsigned char[FileLength];
	fread(FileBuffer, FileLength, 1, fp);
	fclose(fp);
	D3d11Texture* Texture = LoadTextureFromMemory(Name, Device, FileBuffer, FileLength, IsDDS);
	SAFE_DELETE(FileBuffer);
	return Texture;
}

D3d11Texture* TextureManager::LoadTextureFromMemory(std::string Name, ID3D11Device* Device, unsigned char* FileData, int FileLength, bool IsDDS)
{
	if (mTextureArray.find(Name) != mTextureArray.end())
	{
		return mTextureArray[Name];
	}
	ID3D11Resource* resource = NULL;
	ID3D11ShaderResourceView* srv = NULL;
	HRESULT hr = S_OK;
	if (IsDDS)
	{
		hr = CreateDDSTextureFromMemory(Device, FileData, FileLength, &resource, &srv);
	}
	else
	{
		hr = CreateWICTextureFromMemory(Device, FileData, FileLength, &resource, &srv);
	}
	if (FAILED(hr))
	{
		return NULL;
	}
	D3d11Texture* Texture = new D3d11Texture;
	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)(&Texture->mTexture));
	if (FAILED(hr))
	{
		return NULL;
	}
	Texture->mShaderResourceView = srv;
	Texture->SetName(Name);
	SAFE_RELEASE(resource);
	mTextureArray[Name] = Texture;
	return Texture;
}

void TextureManager::DestroyTexture(D3d11Texture* Tex)
{
	std::string Name = Tex->GetName();
	mTextureArray.erase(Name);
	SAFE_DELETE(Tex);
}

bool D3d11Texture::BlitToTexture(unsigned char* Data, int Len)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//  Disable GPU access to the vertex buffer data.
	auto d3dContext = Scene::GetCurrentScene()->GetRenderSystem()->GetD3d11Context();
	HRESULT hr = d3dContext->Map(mTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (hr == S_OK)
	{
		//  Update the vertex buffer here.
		memcpy(mappedResource.pData, Data, Len);
		//  Reenable GPU access to the vertex buffer data.
		d3dContext->Unmap(mTexture, 0);
		return true;
	}
	return false;
}

bool D3d11Texture::Map(BYTE*& lpData, UINT &pitch)
{
	if (!mCpuAccess) return false;
	HRESULT err;
	DXGI_MAPPED_RECT map;
	HRESULT hr = mTexture->QueryInterface(__uuidof(IDXGISurface1), (void **)&mSurface);

	if (FAILED(err = mSurface->Map(&map, DXGI_MAP_READ)))
	{
		OutputDebugStringA(("D3D11Texture::Map: map failed"));
		return false;
	}

	lpData = (BYTE*)map.pBits;
	pitch = map.Pitch;

	return true;
}

void D3d11Texture::Unmap()
{
	mSurface->Unmap();
	SAFE_RELEASE(mSurface);
}