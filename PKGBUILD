# Maintainer:
# Mads Kjeldgaard <mail@madskjeldgaard.dk>
pkgname=birdhouse-git
pkgver=r123.abc1234
pkgrel=1
pkgdesc="Birdhouse CLAP plugin and Standalone version"
arch=('x86_64')
url="https://github.com/madskjeldgaard/Birdhouse"
license=('GPL')
depends=('cmake' 'make' 'gcc' 'alsa-lib' 'freetype2' 'libx11' 'libxinerama' 'juce')
makedepends=('git')
provides=("${pkgname}")
source=("git+${url}.git")
md5sums=('SKIP')

pkgver() {
  cd "${srcdir}/${pkgname}"
  printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
  cd "${srcdir}/${pkgname}"
  mkdir -p build
  cd build
  cmake ..
  make
}

package() {
  cd "${srcdir}/${pkgname}/build"
  make DESTDIR="${pkgdir}" install
}
