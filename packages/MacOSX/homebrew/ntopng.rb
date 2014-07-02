require "formula"

class Ntopng < Formula
  homepage "http://www.ntop.org"
  url 'https://svn.ntop.org/svn/ntop/trunk/ntopng/', :tag => '7829'
  version "1.1.4"

  devel do
     url 'https://svn.ntop.org/svn/ntop/trunk/ntopng/', :tag => '7830'
     version "1.1.4-7830"
  end

  depends_on :autoconf => :build
  depends_on :automake => :build
  depends_on 'pkg-config' => :build
  depends_on 'json-glib' => :build
  depends_on :libtool => :build
  depends_on 'sqlite'
  depends_on 'rrdtool'
  depends_on 'wget'
  depends_on 'libxml2'
  depends_on 'redis'
  depends_on 'glib'
  depends_on 'lbzip2'
  depends_on 'zeromq'

  def install
    system "./autogen.sh"
    system "./configure", "--prefix=#{prefix}", "--mandir=#{man}"
    system "make"
    system "make install"
  end

  test do
    system "true"
  end
end
