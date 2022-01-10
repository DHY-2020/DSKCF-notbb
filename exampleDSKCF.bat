
set topResultsDir=result
set topResultsDirSTRING=./result

set dataDirectory=./ValidationSet

set myDSKCFexe=./DSKCFcpp






::249,238,41,126 

::./DSKCFcpp -b  178,162,121,156  -d  -e result/bear_front/ -o result/bear_front/bear_front.txt -s ValidationSet/bear_front/img/ -i \%.04d.png --depth_sequence ValidationSet/bear_front/depth2/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  246,235,51,128  -d  -e result/zcup_move_1/ -o result/zcup_move_1/zcup_move_1.txt -s ValidationSet/zcup_move_1/img/ -i \%.04d.png --depth_sequence ValidationSet/zcup_move_1/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  166,262,107,207  -d  -e result/new_ex_occ4/ -o result/new_ex_occ4/new_ex_occ4.txt -s ValidationSet/new_ex_occ4/img/ -i \%.04d.png --depth_sequence ValidationSet/new_ex_occ4/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  247,189,73,95  -d  -e result/face_occ5/ -o result/face_occ5/face_occ5.txt -s ValidationSet/face_occ5/img/ -i \%.04d.png --depth_sequence ValidationSet/face_occ5/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  501,272,52,190  -d  -e result/child_no1/ -o result/child_no1/child_no1.txt -s ValidationSet/child_no1/img/ -i \%.04d.png --depth_sequence ValidationSet/child_no1/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  369,217,80,253  -d  -e result/two_people/ -o result/two_people/two_people.txt -s ValidationSet/two_people/img/ -i \%.04d.png --depth_sequence ValidationSet/two_people/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  280,158,95,111  -d  -e result/face_turn2/ -o result/face_turn2/two_people.txt -s ValidationSet/face_turn2/img/ -i \%.04d.png --depth_sequence ValidationSet/face_turn2/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  517,205,76,237  -d  -e result/cf_difficult/ -o result/cf_difficult/cf_difficult.txt -s ValidationSet/cf_difficult/img/ -i \%.04d.png --depth_sequence ValidationSet/cf_difficult/depth/ --depth_image_name_expansion \%.04d.png
::./DSKCFcpp -b  503,163,103,297  -d  -e result/new_ex_occ5/ -o result/new_ex_occ5/new_ex_occ5.txt -s ValidationSet/new_ex_occ5/rgb/ -i \%.04d.png --depth_sequence ValidationSet/new_ex_occ5/depth/ --depth_image_name_expansion \%.04d.png
./DSKCFcpp -b  349,113,84,125  -d  -e result/stereo_recified/ -o result/stereo_recified/stereo_recified.txt -s ValidationSet/stereo_recified/img/ -i \%.05d.png --depth_sequence ValidationSet/stereo_recified/depth/ --depth_image_name_expansion \%.05d.png




