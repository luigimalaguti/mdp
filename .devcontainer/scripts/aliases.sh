alias ..="cd .."
alias ...="cd ../.."
alias -- -="cd -"

alias lah="ls -lah"
function cdl {
    DIR="${1:-$HOME}"
    cd "$DIR" && lah
}
