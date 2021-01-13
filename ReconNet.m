clear, clc
mkdir golden
rng(0);
img_size = 33;

img_in = rand(img_size, img_size)-0.5;
kernel_11 = rand(11,11)-0.5;
kernel_1 = rand(1,1)-0.5;
kernel_7 = rand(7,7)-0.5;

L1 = conv2(img_in, flipud(fliplr(kernel_11)), 'same');  L1 = L1.*(L1 > 0);
L2 = conv2(L1, flipud(fliplr(kernel_1)), 'same');       L2 = L2.*(L2 > 0);
L3 = conv2(L2, flipud(fliplr(kernel_7)), 'same');       L3 = L3.*(L3 > 0);
L4 = conv2(L3, flipud(fliplr(kernel_11)), 'same');      L4 = L4.*(L4 > 0);
L5 = conv2(L4, flipud(fliplr(kernel_1)), 'same');       L5 = L5.*(L5 > 0);
img_out = conv2(L5, flipud(fliplr(kernel_7)), 'same');  img_out = img_out.*(img_out > 0)


writematrix(img_in,'golden/img_in.txt','Delimiter',' ')
writematrix(L1,'golden/L1.txt','Delimiter',' ')
writematrix(L2,'golden/L2.txt','Delimiter',' ')
writematrix(L3,'golden/L3.txt','Delimiter',' ')
writematrix(L4,'golden/L4.txt','Delimiter',' ')
writematrix(L5,'golden/L5.txt','Delimiter',' ')
writematrix(img_out,'golden/img_out.txt','Delimiter',' ')

writematrix(kernel_1,'golden/kernel_1.txt','Delimiter',' ')
writematrix(kernel_7,'golden/kernel_7.txt','Delimiter',' ')
writematrix(kernel_11,'golden/kernel_11.txt','Delimiter',' ')