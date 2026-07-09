alias ..="cd .."
alias ...="cd ../.."
alias -- -="cd -"

alias lah="ls -lah --block-size=MB"
function cdls {
    DIR=$HOME
    if [ $# -eq 1 ]; then
        DIR="$1"
    fi
    cd "$DIR" && lah;
}
alias cd="cdls"
