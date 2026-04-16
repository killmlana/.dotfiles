const yargs = require('yargs/yargs');
const { hideBin } = require('yargs/helpers');
const argv = yargs(hideBin(process.argv)).argv;

const { execSync } = require('child_process');

class SelectionMenu {
  constructor() {
    this.jeson = JSON.parse(execSync('eww -c ~/.config/eww get kbselection').toString());
    this.selected = Object.keys(this.jeson).findIndex(key => this.jeson[key] === 'selected');
  }

  next() {
    const keys = Object.keys(this.jeson);
    if (this.selected + 1 < keys.length) {
      this.jeson[keys[this.selected]] = 'non-selected';
      this.selected += 1;
      this.jeson[keys[this.selected]] = 'selected';
      this.updateShellCommand();
    }
    else {
        this.jeson[keys[this.selected]] = 'non-selected';
        this.selected = 0;
        this.jeson[keys[this.selected]] = 'selected';
        this.updateShellCommand();
    }
  }

  previous() {
    const keys = Object.keys(this.jeson);
    if (this.selected - 1 >= 0) {
      this.jeson[keys[this.selected]] = 'non-selected';
      this.selected -= 1;
      this.jeson[keys[this.selected]] = 'selected';
      this.updateShellCommand();
    }
    else {
        this.jeson[keys[this.selected]] = 'non-selected';
        this.selected = keys.length;
        this.jeson[keys[this.selected]] = 'selected';
        this.updateShellCommand();
    }
  }

  getJeson() {
    return this.jeson;
  }

  updateShellCommand() {
    const command = `eww -c ~/.config/eww update kbselection='${JSON.stringify(this.jeson)}'`;
    execSync(command);
  }
}

const menu = new SelectionMenu();


if (argv._[0] === "next") {
    menu.next();
} else if (argv._[0] === "prev") {
    menu.previous();
}