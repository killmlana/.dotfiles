const { execSync } = require('child_process');

// Read array from the first command line argument
const arrayElements = JSON.parse(process.argv[2]);

// Construct the jeson object
const jeson = arrayElements.reduce((obj, key, i) => {
  obj[key] = i === 0 ? 'selected' : 'non-selected';
  return obj;
}, {});

// Update the variable using the shell command
const serializedJeson = JSON.stringify(jeson);
const command = `eww -c ~/.config/eww update kbselection='${serializedJeson}'`;
console.log(serializedJeson);

execSync(command);