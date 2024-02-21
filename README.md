# GeometryCV

The program recognizes basic geometric shapes from the image and provides their geometric dimensions. 

**Notice:**

The program recognizes shapes only when they are written or printed on grid paper and one of the grid in the right corner must be filled in, as it used to properly scale the objects!

# What version of OpenCV does the program use?

4.6.0

# How to use the program?

Upload your photos in .jpg format to folder "CV". Than add image path to your image in place presented below on screenshot (highlighted in a yellow frame).

![where_upload_imgs](https://github.com/jakubdaron/GeometryCV/assets/102093406/aea16813-820d-4e34-b0b7-989a9af5e6b5)

**Program displays two photos:**

Orginal photo with marked shapes, their dimensions and the reference grid.
![ori_img](https://github.com/jakubdaron/GeometryCV/assets/102093406/976e9eab-990c-4c25-af9f-2801f5c7bff3)

Image after preprocessing that is used by the program to recognise figures and measure them properly.
![Img_after_preprocessing](https://github.com/jakubdaron/GeometryCV/assets/102093406/a52e0d09-82aa-4dc8-b9e0-548da090e8b7)

To change current photos - press "Space", To quit the program - press "Esc". The photos change in sequence according to the defined image path vector.
