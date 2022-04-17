This file should contain:

-You Zhou yz3883, Aoxue Wei aw3389, Panyu Gao pg2676
-HW7
-	Description for each part

Part1 is basically easy. We didn't encounter any big problem. We just inmitated
how the first resource walks the page, and added some error check. The only
confusion is that we are not sure what errno should we return when page walking
fails at middle. Because there is no clear requirement about it on homework page,
and both EINVAL and EFAULT make sense. Finally, we decided to use EFAULT after
long analysis of the part1 requirement.

As the requirement aksed, in part2, all we need to do is replace our page walking
method by get_user_pages_remote(). However, we encoutered two big bug in this part.
Fristly, our program can't handle more than one pages. Each time when we read two
pages, the kernel would panic. We solved it by replacing vmas argument in
get_user_pages_remote() from a empty pointer to a null pointer. Secondly, we can
achieve COW when testing fork program. We solved it by adding the FOLL_WRITE flag
in flags arugment. We infer that only when FOLL_WRITE is set, the GUP will check
whether the page is COW or not.
