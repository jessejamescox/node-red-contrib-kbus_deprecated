##----------------------------------------------------------------------------------------------------------------------
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <https://www.gnu.org/licenses/>.
###
##----------------------------------------------------------------------------------------------------------------------
#
PACKAGES-$(PTXCONF_KBUSPROC) +=kbusproc

#
# Paths and names
#
KBUSPROC_VERSION	:= 0.0.4
KBUSPROC		:= kbusproc
KBUSPROC_URL		:= file://../ipc-dist/src/kbusproc
KBUSPROC_DIR		:= $(BUILDDIR)/$(KBUSPROC)
KBUSPROC_SOURCE		:= $(SRCDIR)/$(KBUSPROC)

CDUP = ..

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(STATEDIR)/kbusproc.get:
	@$(call targetinfo, $@)
	@$(call touch, $@)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

$(STATEDIR)/kbusproc.extract:
	@$(call targetinfo, $@)
	@$(call clean, $(KBUSPROC_DIR))
	cp -rd $(KBUSPROC_SOURCE) $(KBUSPROC_DIR)
	@$(call touch, $@)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

KBUSPROC_PATH	:= PATH=$(CROSS_PATH)
KBUSPROC_ENV 	:= $(CROSS_ENV)

$(STATEDIR)/kbusproc.prepare:
	@$(call targetinfo, $@)
	@$(call touch, $@)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/kbusproc.compile:
	@$(call targetinfo, $@)
	@cd $(KBUSPROC_DIR) && \
		$(KBUSPROC_ENV) $(KBUSPROC_PATH) DIST_DIR=$(PTXDIST_PLATFORMDIR) \
		env \
		DIST_DIR=$(PTXDIST_PLATFORMDIR) CROSS_COMPILE=$(COMPILER_PREFIX) \
		$(MAKE)			
	@$(call touch, $@)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

$(STATEDIR)/kbusproc.install:
	@$(call targetinfo, $@)
	@$(call touch, $@)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/kbusproc.targetinstall:
	@$(call targetinfo, $@)
	@$(call install_init, kbusproc)
	@$(call install_fixup,kbusproc,PRIORITY,optional)
	@$(call install_fixup,kbusproc,VERSION,$(KBUSPROC_VERSION))	
	@$(call install_fixup,kbusproc,SECTION,base)
	@$(call install_fixup,kbusproc,AUTHOR,"cm")
	@$(call install_copy, kbusproc, 0, 0, 0755, $(KBUSPROC_DIR)/kbusproc, /usr/bin/kbusproc)
	@$(call install_finish,kbusproc)
	@$(call touch, $@)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

kbusproc_clean:
	rm -rf $(STATEDIR)/kbusproc.*
	
# vim: syntax=make
