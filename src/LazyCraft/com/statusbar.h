#include "com_interface.h"

#include <list>
#include <memory>
#include <tuple>

namespace com {

struct IStatusBar : public IUtility {
	enum class Alignment { Left, Right };

	virtual const ImRect rect() const	= 0;
	virtual const ImVec2 cursor() const = 0;

	virtual bool prepare(const IUtility* parent) = 0;
	virtual void render() const					 = 0;

	virtual IStatusBar* add_util(IUtility* util, Alignment align) = 0;
};

}	// namespace com

namespace com::impl {

using com::IStatusBar;

struct StatusBar : public IStatusBar {
	virtual const ImRect rect() const override;
	virtual const ImVec2 cursor() const override;

	virtual bool prepare(const IUtility* parent = nullptr) override;
	virtual void render() const override;

	virtual StatusBar* add_util(IUtility* util, Alignment align) override;

private:
	using Utility  = std::unique_ptr<IUtility>;
	using UtilType = std::tuple<Utility, Alignment, bool>;

	float				height_{20.00f};
	Alignment			align_;
	ImVec2				cursor_pos_[2];
	std::list<UtilType> utils_;
};

}	// namespace com::impl