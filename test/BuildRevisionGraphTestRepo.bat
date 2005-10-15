rem @echo off
svnadmin create repo --fs-type fsfs
svn co file:///d:/test/repo wc1
cd wc1
svn mkdir trunk
svn mkdir branches
svn mkdir tags
svn ci -m "create repo layout"

:: revision 1

cd trunk
echo testline1 > file1
svn add file1
echo testline1 > file2
svn add file2
echo testline1 > file3
svn add file3
echo testline1 > file4
svn add file4
svn mkdir folder
echo testline1 > folder\file1
svn add folder\file1
echo testline1 > folder\file2
svn add folder\file2
echo testline1 > folder\file3
svn add folder\file3
echo testline1 > folder\file4
svn add folder\file4
svn ci -m "add some files and a folder with files"

:: revision 2

svn cp file:///d:/test/repo/trunk file:///d:/test/repo/branches/TestBranch1 -m "create test branch"

:: revision 3

echo testline > file1
svn ci -m "file modified"

:: revision 4

svn ren folder Ordner
svn ci -m "rename a folder"

:: revision 5

svn cp file:///d:/test/repo/trunk file:///d:/test/repo/tags/V1 -m "Version 1"

:: revision 6

svn cp file:///d:/test/repo/tags/V1 file:///d:/test/repo/branches/RelV1 -m "Version 1 Fix branch"

:: revision 7

svn cp file:///d:/test/repo/trunk -r2 file:///d:/test/repo/tags/V0 -m "Version 0"

:: revision 8

svn up
echo testline2 > file1
svn ci -m ""
:: revision 9
echo testline3 > file1
svn ci -m ""
:: revision 10
echo testline4 > file1
svn ci -m ""
:: revision 11
echo testline5 > file1
svn ci -m ""
:: revision 12
echo testline6 > file1
svn ci -m ""
:: revision 13
echo testline7 > file1
svn ci -m ""
:: revision 14
echo testline8 > file1
svn ci -m ""
:: revision 15
echo testline9 > file1
svn ci -m ""
:: revision 16
svn cp file:///d:/test/repo/trunk -r10 file:///d:/test/repo/tags/V2 -m "Version 2"

:: revision 17

svn cp file:///d:/test/repo/trunk -r9 file:///d:/test/repo/tags/V2 -m "Version 1.5"

:: revision 18

svn rm file:///d:/test/repo/tags/V2/trunk -m ""

:: revision 19

svn cp file:///d:/test/repo/trunk -r9 file:///d:/test/repo/tags/V1.5 -m "Version 1.5"

:: revision 20

svn cp file:///d:/test/repo/trunk file:///d:/test/repo/branches/newtest -m "new test"

:: revision 21

svn rm file:///d:/test/repo/branches/newtest -m "remove wrong branch"

:: revision 22

svn cp file:///d:/test/repo/trunk -r16 file:///d:/test/repo/branches/newtest -m "new test"

:: revision 23

svn mkdir Ordner2
svn ci -m "new folder"

:: revision 24

svn rm Ordner
svn ci -m "remove folder"

:: revision 25

svn mv Ordner2 Ordner
svn ci -m "move folder"

:: revision 26

echo testline > Ordner\file1
svn add Ordner\file1
svn ci -m "add file"

:: revision 27

echo testline1 > Ordner\file1
svn ci -m "mod"

:: revision 28

svn up
svn rm Ordner
svn ci -m ""

:: revision 29

svn cp file:///d:/test/repo/trunk/Ordner -r24 Ordner
svn ci -m "restore original"

:: revision 30

svn cp Ordner Ordner2
svn cp Ordner Ordner3
svn cp Ordner Ordner4
svn mv file1 Ordner3/mvfile
svn up
svn ci -m "mess up"

:: revision 31



cd ..
cd ..
