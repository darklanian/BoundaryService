//
// Created by kwangil.ji on 2022-05-10.
//

#ifndef BOUNDARYSERVICE_BOUNDARY_H
#define BOUNDARYSERVICE_BOUNDARY_H

void boundary_init(AAssetManager* am);
void boundary_deinit();
void boundary_draw_grid(XrMatrix4x4f vp);
void boundary_draw_surface(XrMatrix4x4f vp);

#endif //BOUNDARYSERVICE_BOUNDARY_H
