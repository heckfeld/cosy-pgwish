# test BLT_RND und UROUND aus bltGrAxis.c
BEGIN {
    OFMT="%.30g";
}
function BLT_RND(x) {
    return (int((x) + (((x)<0.0) ? -0.5 : 0.5)));
}
function UROUND(x,u) {
    return (BLT_RND((x)/(u))*(u));

}
{
	print $1,$2,UROUND( $1, $2);
}
END {
}
