pkgname="bakatui"
_pkgname="bakatui"
provides=("${_pkgname}")
conflicts=("${_pkgname}")
pkgver=1.0.3.r0.gab854ed
pkgrel=1
arch=('i686' 'pentium4' 'x86_64' 'arm' 'armv6h' 'armv7h' 'aarch64' 'riscv64')
url=https://git.pupes.org/PoliEcho/bakatui
pkgdesc="bakalari for your terminal"
license=('GPL-3.0-or-later')
depends=(
	'curl'
	'ncurses'
	)
makedepends=('nlohmann-json'
	     'curl'
            )
source=("git+${url}.git")
sha256sums=('SKIP')


pkgver() {
    cd "$srcdir/$_pkgname"
    git fetch --tags 2>/dev/null
    
    local _tag=$(git tag -l 'v[0-9]*' | grep -E '^v[0-9]+\.[0-9]+' | sort -V | tail -n1)
    
    if [ -z "$_tag" ]; then
        echo "0.0.0.r$(git rev-list --count HEAD).$(git rev-parse --short HEAD)"
    else
        git describe --long --tags "$_tag" | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g'
    fi
}


prepare() {
    cd "$srcdir/$_pkgname"
    
    local _tag=$(git tag -l 'v[0-9]*' | grep -E '^v[0-9]+\.[0-9]+' | sort -V | tail -n1)
    
    if [ -n "$_tag" ]; then
        echo "Checking out tag: $_tag"
        git checkout "$_tag"
    else
        echo "Warning: No version tags found, using HEAD"
    fi
}


build() {
    cd "$srcdir/$_pkgname"
    
    make
}

package() {
    cd "$srcdir/$_pkgname"
    mkdir -p "${pkgdir}/usr/bin"
    make DESTDIR="$pkgdir" install
    
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
