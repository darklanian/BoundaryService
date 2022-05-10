//
// Created by kwangil.ji on 2022-05-10.
//

#include <android_native_app_glue.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void load_texture(AAssetManager* am, const char* filename, GLuint texture, GLenum textureTarget) {
	AAsset* asset = AAssetManager_open(am, filename, AASSET_MODE_BUFFER);
	int width, height, channels;
	//stbi_set_flip_vertically_on_load(1);
	uint8_t* pixels = stbi_load_from_memory((uint8_t *)AAsset_getBuffer(asset), AAsset_getLength(asset), &width, &height, &channels, 4);

	glBindTexture(textureTarget, texture);

	glTexImage2D(textureTarget, 0,  // mip level
				 GL_RGBA,
				 width, height,
				 0,                // border color
				 GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT );

	stbi_image_free(pixels);

	AAsset_close(asset);
}