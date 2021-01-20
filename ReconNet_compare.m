clear, clc
% Test image
% [test, map] = imread('test_images/barbara.tif');
% fid = fopen('golden/barbara_in.txt','w+');
% [row_img, col_img] = size(test); 
% for i = 1:row_img
%     for j = 1:col_img
%         fprintf(fid,'%f\n',test(i,j));
%     end
% end
% fclose(fid);

% golden image
% [golden, map] = imread('original_images/barbara.tif');
% fid = fopen('golden/barbara_out.txt','w+');
% [row_img, col_img] = size(golden); 
% for i = 1:row_img
%     for j = 1:col_img
%         fprintf(fid,'%f\n',golden(i,j));
%     end
% end
% fclose(fid);

% golden image w/ normalization for testbench use
% [golden_0_1, map] = imread('original_images/barbara.tif');
% fid = fopen('golden/barbara_out_0_1.txt','w+');
% [row_img, col_img] = size(golden_0_1); 
% min_pix = min(golden_0_1,[],'all');
% max_pix = max(golden_0_1,[],'all');
% golden_0_1 = (double(golden_0_1)-double(min_pix))/(double(max_pix)-double(min_pix));
% for i = 1:row_img
%     for j = 1:col_img
%         fprintf(fid,'%f\n',golden_0_1(i,j));
%     end
% end
% fclose(fid);
img_pic_array = [256, 256]; 
[fid_matlab, message_matlab] = fopen('golden/barbara_out_0_1.txt', 'r');
img_pic_matlab = fscanf(fid_matlab,'%f',prod(img_pic_array,'all'));
img_pic_matlab = reshape(img_pic_matlab, img_pic_array);
img_pic_matlab = permute(img_pic_matlab, [2 1]);
subplot(1,3,1);
imshow(img_pic_matlab);
title(['float16 from matlab']);  
% img_pic_array = [256, 256]; 
[fid, message] = fopen('D:/MSOC109-1/final_project/test/ReconNet-hls-master/hls_ReconNet/solution8_test_correct_input/csim/build/barbara_out_csim.txt', 'r');
img_pic = fscanf(fid,'%f',prod(img_pic_array,'all'));
img_pic = reshape(img_pic, img_pic_array);
img_pic = permute(img_pic, [2 1]);
subplot(1,3,2);
imshow(img_pic);
title(['float16 csim']);  
subplot(1,3,3);
real = imread('original_images/barbara.tif');
imshow('original_images/barbara.tif'); 
title('real');




function psnr = psnr1(img1, img2)
   diff = img1 - img2;
%    disp(diff);
   mse = mean(diff.^2, 'all');
%    disp(mse);
   if mse < 1.0e-10
      psnr = 100;
   
   else
       psnr = 10 * log10(255.0*255.0/mse);
   end
end